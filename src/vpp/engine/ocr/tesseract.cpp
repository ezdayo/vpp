/**
 *
 * @file      vpp/engine/ocr/tesseract.cpp
 *
 * @brief     This is the Tesseract OCR engine
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#include <locale>
#include <string>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>

#include "vpp/log.hpp"
#include "vpp/engine/ocr/tesseract.hpp"

namespace VPP {
namespace Engine {
namespace OCR {

/* Helper function to clean what has been OCR'ed */
static void insertUTF8Text(Zone &zone, const char *info) {
    zone.description.clear();

    if (strlen(info) > 0) {
        zone.description.resize(strlen(info)+1);
        // From (f) and To (t) pointers
        auto f = (uint8_t *) (info);
        auto s = (uint8_t *) (zone.description.data());
        auto t = s;
 
        // Start of text (sot) and was separator (sep) tags
        bool sot=true;
        bool sep=false;

        while (true) {

            /* Keep letters, numbers and punctuation */
            if ( ((*f >= '!') && (*f <= 'Z')) ||
                 ((*f >= 'a') && (*f <= 'z')) ) {
                if ((!sot)  && (sep)) ++t;
                *t = *f;
                ++f; ++t;
                sot=false; sep=false;
                continue;
            }

            /* Manage the space (and avoid multiple spaces) */
            if (*f == ' ') {
                ++f;
                if ((!sot) && (!sep)) {
                    *t = ' ';
                }
                sep=true;
                continue;
            }

            /* Manage carriage returns */
            if (*f == '\n') {
                ++f;
                if (!sot) {
                    *t='\n';
                }
                sep=true;
                continue;
            }

            /* Exit if '\0' */
            if (*f == '\0') {
                *t='\0';
                break;
            }

            /* UTF-8 extensions keep all accentued characters and money signs */
            if (*f == 0xc3) {
                if ((!sot)  && (sep)) ++t;
                *t=*f; ++f; ++t;
                *t=*f; ++f; ++t;
                sot=false; sep=false;
                continue;
            }

            if (*f == 0xc2) {
                switch (*(f+1)) {
                    /* Keep the following characters */
                    case 0xa2: // ¢
                    case 0xa3: // £
                    case 0xa5: // ¥
                    case 0xaa: // «
                    case 0xbb: // »
                        if ((!sot)  && (sep)) ++t;
                        *t=*f; ++f; ++t;
                        *t=*f; ++f; ++t;
                        sot=false; sep=false;
                        break;
                    /* Skip other characters */
                    default:
                        f+=2;
                }
                continue;
            }

            /* Skip any other character */
            ++f;
        }

        zone.description.resize(t-s);
    } else {
        zone.description = "";
    }
}

Tesseract::Tesseract() noexcept
    : path(""), language(""), current_path(""), current_language(""), 
      current_oem(tesseract::OEM_COUNT),
      current_psm(tesseract::PSM_COUNT), tess() {

        path.denominate("path")
            .describe("The path for all Tesseract OCR configuration files")
            .characterise(Customisation::Trait::CONFIGURABLE);
        expose(path);

        language.denominate("language")
                .describe("The language for the Tesseract OCR")
                .characterise(Customisation::Trait::CONFIGURABLE);
        expose(language);

        oem.denominate("oem")
           .describe("Tesseract OCR Engine Mode")
           .characterise(Customisation::Trait::CONFIGURABLE);
        oem.range(0, tesseract::OEM_COUNT-1);
        expose(oem);
        oem = static_cast<int>(tesseract::OEM_LSTM_ONLY);

        psm.denominate("psm")
           .describe("Tesseract Page Segmentation Mode")
           .characterise(Customisation::Trait::CONFIGURABLE);
        psm.range(0, tesseract::PSM_COUNT-1);
        expose(psm);
        psm = static_cast<int>(tesseract::PSM_AUTO_OSD);
}

Customisation::Error Tesseract::setup() noexcept {
    auto req_oem = static_cast<tesseract::OcrEngineMode>(static_cast<int>(oem));
    auto req_psm = static_cast<tesseract::PageSegMode>(static_cast<int>(psm));

    if ( (!static_cast<std::string>(language).empty()) &&
         (static_cast<std::string>(path) == current_path) &&
         (static_cast<std::string>(language) == current_language) &&
         (req_oem == current_oem) &&
         (req_psm == current_psm) ) {
        return Customisation::Error::NONE;
    }

    terminate();
    auto error = tess.Init(static_cast<std::string>(path).c_str(),
                           static_cast<std::string>(language).c_str(),
                           req_oem);

    if (error) {
        LOGE("%s[%s]::setup(): Tesseract initialisation error %d",
             value_to_string().c_str(), name().c_str(), error);
        return Customisation::Error::INVALID_VALUE;
    }

    tess.SetVariable("debug_file", "/dev/null");
    tess.SetPageSegMode(req_psm);

    current_path     = path;
    current_language = language;
    current_oem      = req_oem;
    current_psm      = req_psm;

    return Customisation::Error::NONE;
}

Error::Type Tesseract::process(Scene &scene, Zone &zone) noexcept {
    ASSERT( ((zone.width > 0) || (zone.height > 0) ),
            "%s[%s]::process(): Invalid BBOX provided : %d x %x at (%d, %d)",
            value_to_string().c_str(), name().c_str(),
            zone.width, zone.height, zone.x, zone.y);

    const cv::Mat &input = scene.view.bgr().input();
    cv::Mat text = input(zone);

    tess.SetImage(text.data, text.cols, text.rows, 3, text.step);

    auto info = tess.GetUTF8Text();
    insertUTF8Text(zone, info);
    delete[] info;

    return Error::NONE;
}

void Tesseract::terminate() noexcept {
    if (!current_language.empty()) {
        current_language.clear();
        current_path.clear();
        tess.End();
    }
}

}  // namespace OCR
}  // namespace Engine
}  // namespace VPP
