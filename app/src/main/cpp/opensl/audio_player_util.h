//
// Created by chengjunsen on 2018/10/19.
//

#ifndef SPLAYER_AUDIO_PLAYER_UTIL_H
#define SPLAYER_AUDIO_PLAYER_UTIL_H
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

namespace AudioPlayerUtil {
    int getChannel(int channels) {
        switch (channels) {
            case 1:
                return SL_SPEAKER_FRONT_CENTER;
            case 2:
                return SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
            default:
                return SL_SPEAKER_FRONT_CENTER;
        };
    }

    int getSampleRate(int sampleRate) {
        switch (sampleRate) {
            case 8000:
                return SL_SAMPLINGRATE_8;
            case 11025:
                return SL_SAMPLINGRATE_11_025;
            case 12000:
                return SL_SAMPLINGRATE_12;
            case 16000:
                return SL_SAMPLINGRATE_16;
            case 22050:
                return SL_SAMPLINGRATE_22_05;
            case 24000:
                return SL_SAMPLINGRATE_24;
            case 32000:
                return SL_SAMPLINGRATE_32;
            case 44100:
                return SL_SAMPLINGRATE_44_1;
            case 48000:
                return SL_SAMPLINGRATE_48;
            case 64000:
                return SL_SAMPLINGRATE_64;
            case 88200:
                return SL_SAMPLINGRATE_88_2;
            case 96000:
                return SL_SAMPLINGRATE_96;
            case 192000:
                return SL_SAMPLINGRATE_192;
            default:
                return SL_SAMPLINGRATE_44_1;
        }
    }

    const char * getErrorString(SLresult result) {
        switch (result) {
            case SL_RESULT_PRECONDITIONS_VIOLATED:
                return "Preconditions violated";
            case SL_RESULT_PARAMETER_INVALID:
                return "Invalid parameter";
            case SL_RESULT_MEMORY_FAILURE:
                return "Memory failure";
            case SL_RESULT_RESOURCE_ERROR:
                return "Resource error";
            case SL_RESULT_RESOURCE_LOST:
                return "Resource lost";
            case SL_RESULT_IO_ERROR:
                return "IO error";
            case SL_RESULT_BUFFER_INSUFFICIENT:
                return "Insufficient buffer";
            case SL_RESULT_CONTENT_CORRUPTED:
                return "Content corrupted";
            case SL_RESULT_CONTENT_UNSUPPORTED:
                return "Content unsupported";
            case SL_RESULT_CONTENT_NOT_FOUND:
                return "Content not found";
            case SL_RESULT_PERMISSION_DENIED:
                return "Permission denied";
            case SL_RESULT_FEATURE_UNSUPPORTED:
                return "Feature unsupported";
            case SL_RESULT_INTERNAL_ERROR:
                return "Internal error";
            case SL_RESULT_UNKNOWN_ERROR:
                return "Unknown error";
            case SL_RESULT_OPERATION_ABORTED:
                return "Operation aborted";
            case SL_RESULT_CONTROL_LOST:
                return "Control lost";
            default:
                return "Unknown OpenSL error";
        }
    }

    const char* ResultToString(SLresult result) {
        const char* str = 0;

        switch (result) {
            case SL_RESULT_SUCCESS:
                str = "Success";
                break;

            case SL_RESULT_PRECONDITIONS_VIOLATED:
                str = "Preconditions violated";
                break;

            case SL_RESULT_PARAMETER_INVALID:
                str = "Parameter invalid";
                break;

            case SL_RESULT_MEMORY_FAILURE:
                str = "Memory failure";
                break;

            case SL_RESULT_RESOURCE_ERROR:
                str = "Resource error";
                break;

            case SL_RESULT_RESOURCE_LOST:
                str = "Resource lost";
                break;

            case SL_RESULT_IO_ERROR:
                str = "IO error";
                break;

            case SL_RESULT_BUFFER_INSUFFICIENT:
                str = "Buffer insufficient";
                break;

            case SL_RESULT_CONTENT_CORRUPTED:
                str = "Success";
                break;

            case SL_RESULT_CONTENT_UNSUPPORTED:
                str = "Content unsupported";
                break;

            case SL_RESULT_CONTENT_NOT_FOUND:
                str = "Content not found";
                break;

            case SL_RESULT_PERMISSION_DENIED:
                str = "Permission denied";
                break;

            case SL_RESULT_FEATURE_UNSUPPORTED:
                str = "Feature unsupported";
                break;

            case SL_RESULT_INTERNAL_ERROR:
                str = "Internal error";
                break;

            case SL_RESULT_UNKNOWN_ERROR:
                str = "Unknown error";
                break;

            case SL_RESULT_OPERATION_ABORTED:
                str = "Operation aborted";
                break;

            case SL_RESULT_CONTROL_LOST:
                str = "Control lost";
                break;

            default:
                str = "Unknown code";
        }

        return str;
    }
}

#endif //SPLAYER_AUDIO_PLAYER_UTIL_H
