# Introduction to libavtranscoder
This is an android lib project with NDK developing.You can use it to transcode videos or decode_encode videos.FFMPEG is the most important dependence for this project.All the transcode
functions powered by FFMPEG.   

## Modules in libavtranscoder
![image1](https://md-image-bucket.oss-cn-guangzhou.aliyuncs.com/github_project_avtranscoder.png)
   
* Decoder    
hw_decode or sw_decode.   

* Encoder    
hw_encode(need JNI call JAVA API in native) or sw_encode.    

* [`continue...`]()


## Transcdoe work flow    
![image2](https://md-image-bucket.oss-cn-guangzhou.aliyuncs.com/github_project_avtranscoder_work_flow_1.png)

 [`continue...`]()
 
## 
# About FFMPEG n4.1.6
FFMPEG n4.1.6 support Android hw_decode ability,and it can use android mediacodec to decode
video.About how to build FFMPEG for android , see [this][1] [`continue...`]() in my repostory.


 [`continue...`]()

[1]:https://www.zhihu.com/question/23378396