/**
 * IZG - Zaklady pocitacove grafiky - FIT VUT
 * Lab 01 - Redukce barevneho prostoru
 *
 * Copyright (c) 2022 by Michal Vlnas, ivlnas@fit.vutbr.cz
 *
 * Tato sablona slouzi vyhradne pro studijni ucely, sireni kodu je bez vedomi autora zakazano.
 */

#include "base.h"

const int matrix[] ={
    0, 204, 51, 255,
    68, 136, 187, 119,
    34, 238, 17, 221,
    170, 102, 153, 85
};

void ImageTransform::grayscale()
{
    for(unsigned int x = 0; x < cfg->width; x++){
        for(unsigned int y = 0; y < cfg->height; y++){
            RGB temp = getPixel(x, y);
            RGB grey_value;
            int i = std::round(temp.r * 0.299 + temp.g * 0.587 + temp.b * 0.114);
            grey_value.r = grey_value.g = grey_value.b = i;
            setPixel(x, y, grey_value);
        }
    }
}

void ImageTransform::threshold()
{
    grayscale();
    for(unsigned int x = 0; x < cfg->width; x++){
        for(unsigned int y = 0; y < cfg->height; y++){
            RGB temp = getPixel(x, y);
            if(temp.r > cfg->threshold) setPixel(x, y, COLOR_WHITE);
            else setPixel(x, y, COLOR_BLACK);
        }
    }
}

void ImageTransform::randomDithering()
{
    grayscale();

    for (uint32_t y = 0; y < cfg->height; y++)
    {
        for (uint32_t x = 0; x < cfg->width; x++)
        {
            auto p = getPixel(x, y);
            uint8_t value = p.r > getRandom() ? 255 : 0;

            setPixel(x, y, RGB(value));
        }
    }
}

void ImageTransform::orderedDithering()
{
    grayscale();
    for(unsigned int x = 0; x < cfg->width; x++){
        for(unsigned int y = 0; y < cfg->height; y++){
            RGB temp = getPixel(x, y);
            int i = x % m_side;
            int j = y % m_side;
            int mp = matrix[(j*m_side) + i];
            
            if(temp.r > mp) setPixel(x, y, COLOR_WHITE);
            else setPixel(x, y, COLOR_BLACK);
        }
    }
}

void ImageTransform::updatePixelWithError(uint32_t x, uint32_t y, float err)
{
    int new_color;
    if(x+1 < cfg->width){
        RGB right = getPixel(x+1, y);

        new_color = right.r + round((3.0/8.0)*err);
        if(new_color > 255) new_color = 255;
        else if(new_color < 0) new_color = 0;
        right.g = right.b = right.r = new_color;
        setPixel(x+1, y, right);
    }

    if(y+1 < cfg->height){
        RGB down = getPixel(x, y+1);

        new_color = down.r + round((3.0/8.0)*err);
        if(new_color > 255) new_color = 255;
        else if(new_color < 0) new_color = 0;
        down.r = down.b = down.g = new_color;
        setPixel(x, y+1, down);
    }

    if(x+1 < cfg->width && y+1 < cfg->height){
        RGB corner = getPixel(x+1, y+1);

        new_color = corner.r + round((1.0/4.0)*err);
        if(new_color > 255) new_color = 255;
        else if(new_color < 0) new_color = 0;
        corner.r = corner.b = corner.g = new_color;
        setPixel(x+1, y+1, corner);
    }
}

void ImageTransform::errorDistribution()
{
    grayscale();
    for(unsigned int y = 0; y < cfg->height; y++){
        for(unsigned int x = 0; x < cfg->width; x++){
            RGB temp = getPixel(x, y);
            float E;

            if(temp.r > cfg->threshold){
                E = temp.r - COLOR_WHITE.r;
                setPixel(x, y, COLOR_WHITE);
            } 
            else{
                E = temp.r;
                setPixel(x, y, COLOR_BLACK);
            }
            updatePixelWithError(x, y, E);
        }
    }
}

