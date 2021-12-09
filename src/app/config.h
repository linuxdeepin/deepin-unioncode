// config.h.in platfrom plugin output
// set(DFM_PLUGIN_PATH) in cmake
// if setting new definition, please use:
//  #undef DFM_PLUGIN_PATH
//  #define DFM_PLUGIN_PATH xxx
#ifndef PLUGIN_PATH
#define PLUGIN_PATH "/usr/lib/x86_64-linux-gnu/unioncode/plugins"
#elif
    #warning "cmake unseting definition PLUGIN_PATH"
#endif

#ifndef BUILD_OUT_PLGUN_DIR
#define BUILD_OUT_PLGUN_DIR "/home/workspace/recode/unioncode/build/Debug"
#elif
    #warning "cmake unseting definition DFM_BUILD_OUT_PLGUN_DIR"
#endif
