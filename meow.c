#include "meow.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <zbar.h>
#include <wand/MagickWand.h>

inline int dump_error(MagickWand *wand)
{
    int exit_code = 0;
    char *desc;
    ExceptionType severity;
    desc = MagickGetException(wand, &severity);

    if(severity >= FatalErrorException)
        exit_code = 2;
    else if(severity >= ErrorException)
        exit_code = 1;
    else
        exit_code = 0;

    const char *sevdesc[] = { "WARNING", "ERROR", "FATAL" };
    fprintf(stderr, "%s: %s\n", sevdesc[exit_code], desc);

    MagickRelinquishMemory(desc);
    return(exit_code);
}

const char *scan_image (const char *filename, zbar_processor_t *processor)
{
    char *output;
    MagickWand *images = NewMagickWand();
    if(!MagickReadImage(images, filename) && dump_error(images))
        return NULL;

    unsigned seq, n = MagickGetNumberImages(images);
    for(seq = 0; seq < n; seq++) {
        if(!MagickSetIteratorIndex(images, seq) && dump_error(images))
            return NULL;

        zbar_image_t *zimage = zbar_image_create();
        assert(zimage);
        zbar_image_set_format(zimage, *(unsigned long*)"Y800");

        int width = MagickGetImageWidth(images);
        int height = MagickGetImageHeight(images);
        zbar_image_set_size(zimage, width, height);

        // extract grayscale image pixels
        // FIXME color!! ...preserve most color w/422P
        // (but only if it's a color image)
        size_t bloblen = width * height;
        unsigned char *blob = malloc(bloblen);
        zbar_image_set_data(zimage, blob, bloblen, zbar_image_free_data);

        if(!MagickExportImagePixels(images, 0, 0, width, height,
                                    "I", CharPixel, blob))
            return NULL;

        zbar_process_image(processor, zimage);

        // output result data
        const zbar_symbol_t *sym = zbar_image_first_symbol(zimage);
        for(; sym; sym = zbar_symbol_next(sym)) {
            zbar_symbol_type_t typ = zbar_symbol_get_type(sym);
            if(typ == ZBAR_PARTIAL)
                continue;
            else
                // printf("%s%s:%s\n",
                       // zbar_get_symbol_name(typ),
                       // zbar_get_addon_name(typ),
                       // zbar_symbol_get_data(sym));
                // TODO, there may be more than 1 symbol.
                return zbar_symbol_get_data(sym);
        }

        zbar_image_destroy(zimage);
    }

    DestroyMagickWand(images);
    return output;
}

void start() {
    MagickWandGenesis();
}

void finish() {
    MagickWandTerminus();
}

const char* convert_teh_stuff(const char *filename) {
    zbar_processor_t *processor = zbar_processor_create(0);
    assert(processor);
    if(zbar_processor_init(processor, NULL, 0)) {
        zbar_processor_error_spew(processor, 0);
        return NULL;
    }

    const char *output = scan_image(filename, processor);

    zbar_processor_destroy(processor);
    return(output);
}
