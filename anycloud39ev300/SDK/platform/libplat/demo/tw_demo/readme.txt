anyka tone wave demo
本示例程序介绍：
1、智能声控识别，并将解析出来放入输出缓存。如果用于声波对码，可以在获取到解析后的ssid和passwd之后调用设置网络的接口。
2、智能声控wav生成，可将ssid和passwd形成声控字符串，生成后的数据可用于手机或PC端播放。可直接将生成接口与对应的库，集成到手机app端。

3、调用示例：
3.1、生成智能声控wav音频：
	./ak_tw_demo ssid password save_wav_file
	本示例程序使用无线路由的SSID与密码，生成对应的wave格式文件。

3.2、智能声控识别：
	./ak_tw_demo
	示例程序中设置的超时时间为60秒