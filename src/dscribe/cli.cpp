/**
 *
 * @file      dscribe/cli.cpp
 *
 * @brief     This is the top-level d-scribe command line interface (CLI)
 *
 * @details   This is the implementation of a command line interface for the
 *            D-Scribe example. It uses the CLI capabilities of the
 *            Customisation framework and provides two visualisation windows
 *            (one after the detection pipeline and the other after the
 *            classification pipeline).
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

#include <cstdio>
#include <opencv2/core.hpp>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <unistd.h>

#include "customisation.hpp"
#include "dscribe/pipeline.hpp"
#include "vpp/log.hpp"

using VPP::Scene;
using VPP::Zone;

static void onScene(const Scene &scn, int error) {
    if (error) {
        LOGE("OOOPS! Error %d on scene '%08lx'! This shall never happen...",
             error, scn.timestamp());
    } else {
        cv::imshow("detection", scn.output());
        cv::waitKey(1);
    }
}

static void onZone(DScribe::Core &dscribe, 
                   const Scene &scn, const Zone &z, int error) {
    if (error) {
        LOGE("OOOPS! Error %d on zone '%s'! This shall never happen...",
             error, z.description.c_str());
    } else {
        if (dscribe.classification.input.bridge.empty()) {
            cv::imshow("classification", scn.output());
            cv::waitKey(1);
        }
    }
}

int main(int argc, char **argv) {
#ifdef CUSTOMISATION_HAS_CLI
    DScribe::Core dscribe;
    Customisation::CLI cli(dscribe);

    /* Remove all kind of messages */
    FILE *stdnull = fopen("/dev/null", "w");
    STDE = stderr;
    STDW = stdout;
    STDO = stdout;
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    
    cv::namedWindow("detection", cv::WINDOW_KEEPRATIO);
    cv::namedWindow("classification", cv::WINDOW_KEEPRATIO);

    /* Forward the detected scene to the classification pipeline */
    dscribe.detection.finished = 
        [&dscribe] (Scene &s) {
        if (!s.zones().empty()) {
            dscribe.classification.input.bridge.forward(std::move(s));
            dscribe.classification.input.bridge.forward(
                std::move(dscribe.classification.input.bridge.scene().zones()));
            dscribe.detection.freeze();
            dscribe.classification.start();
        } };

    dscribe.classification.finished = 
        [&dscribe] (Scene &/*s*/, Zone &/*z*/) {
            if (dscribe.classification.input.bridge.empty()) {
                dscribe.detection.unfreeze();
            }
        };

    dscribe.detection.broadcast.connect(onScene);
    dscribe.classification.broadcast.connect([&dscribe](const Scene &scn,
                                                        const Zone &z, 
                                                        int error) { 
                return onZone(dscribe, scn, z, error); } );
   
    if (argc <= 1) { 
        cli.interactive();
    } else {
        for (int i=1; i < argc; ++i) {
            cli.script(argv[i]);
        }
    }
    dscribe.finalise();
    fclose(stdnull);

#else
    LOGE("Need to configure the Customisation library with its CLI");
#endif

    return 0;
}
