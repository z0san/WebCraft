//
// Created by CHOI Jongmin on 13/11/2021.
//

#ifndef WEBCRAFT_SMARTPOINTERHELP_H
#define WEBCRAFT_SMARTPOINTERHELP_H

#include <memory>

// A collection of preprocessor definitions to
// save time in writing out smart pointer syntax
#define uPtr std::unique_ptr
#define mkU std::make_unique
// Rewrite std::unique_ptr<float> f = std::make_unique<float>(5.f);
// as uPtr<float> f = mkU<float>(5.f);

#define sPtr std::shared_ptr
#define mkS std::make_shared
// Rewrite std::shared_ptr<float> f = std::make_shared<float>(5.f);
// as sPtr<float> f = mkS<float>(5.f);

#endif //WEBCRAFT_SMARTPOINTERHELP_H
