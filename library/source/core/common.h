//
// Created by mapgoo1426 on 2022/11/4.
//

#ifndef ANDROID_LIBTRANSCODE_COMMON_H
#define ANDROID_LIBTRANSCODE_COMMON_H


//void SetCallBack(video_frame_call_back callback);
typedef void (*video_frame_call_back)(int index,void *data,int len,unsigned long long pts,int is_key_frame,int nal_type);

#endif //ANDROID_LIBTRANSCODE_COMMON_H
