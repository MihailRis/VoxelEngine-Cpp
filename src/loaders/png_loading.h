/*
 * png_loading.h
 *
 *  Created on: Feb 10, 2020
 *      Author: MihailRis
 */

#ifndef LOADERS_PNG_LOADING_H_
#define LOADERS_PNG_LOADING_H_

#include <string>

class Texture;

extern Texture* load_texture(std::string filename);

#endif /* LOADERS_PNG_LOADING_H_ */
