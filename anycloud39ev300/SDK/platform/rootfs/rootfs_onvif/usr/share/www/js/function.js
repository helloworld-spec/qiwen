
//initial
var rtsp;
var brower_type="ie";
var bFullScreen = false;
var g_usr;
var	g_pwd;
var dvr_url;
var g_ip;
var g_port;
var tmp_ip
var __ipcam_display_content = false;

//video

var dvr_ajax;
//image
var slider_sha;
var slider_hue;
var slider_con;
var slider_lum;
var slider_sat;
var sharpen=0;
var hue=0;
var saturation=0;
var brightness=0;
var contrast=0;
var disp_delaytime_ms = 5000;
var hide_delaytime_ms = 8000;
var g_model = "";

//isp
var slider_str;
var slider_AEcom;
var slider_denoise_strength;
var strength=0;
var ae_compensation=0;
var denoise_strength=0;
//Overlay
var nowChannl = 101;  //默认当前通道为101;
var dateX,dateY,nameX,nameY,deviceX ,deviceY;      //获取Overlay当前百分比

var m_iCurWeekDay=0;

/*if(Cookies.get("usr") == null && Cookies.get("pwd") == null)
	{
       top.location='login.html';
	}*/
//cover
var oWarp,oMove_view;
var oMove = [];  

var base64 = new Base64();

function flash_video(_video_id, _stream_name)
{

	var obj = document.getElementById(_video_id);
	if(obj && obj.ConnectRTMP)
	{
		obj.SetPlayerNum(1);
		obj.SetPoster(0, dvr_url+"/snapshot?chn=1&u=" + g_usr + "&p=" + g_pwd + "&q=0&d=1&rand=&r=" + Math.random());
		obj.ConnectRTMP(0, "rtmp://"+g_ip + ":" + g_port,_stream_name);
		
//		obj.SetBufferTime(0.05);
	}
	else
	{
		setTimeout("flash_video(\"" + _video_id + "\", \"" + _stream_name + "\")", 1000);
	}

}

function del_li(parent_id, n)
 {
  var s=document.getElementById(parent_id);
  var t=s.childNodes.length;
  for (var i=t-1;i>0;i--)
  {
   if (i==n-1)
   {
    s.removeChild(s.childNodes[i]);
   }
  }
 }

$(document).ready(function(){
	tmp_ip=document.location.hostname;
	g_ip=document.location.hostname;
	var tmp_port=document.location.port;
	renewtime();
	if(tmp_port == "")	//80
	{
		tmp_port = 80;
	}
	else
	{
		var tmp_index = document.location.host.indexOf(":");
		tmp_ip = document.location.host.substring(0, tmp_index);
	}
	
	dvr_url = "http://" + tmp_ip +  ":" + tmp_port;//http://192.168.2.45:80
	g_usr = Cookies.get("usr");
	g_pwd = Cookies.get("pwd");
	var bEnable_sync_time = Cookies.get("sync_time");
	if(bEnable_sync_time == 'true'){
		sync_pc_time();
	}
	load1();
	image_load_content();
});

function flv_view(streamtype)
{
	if(__ipcam_display_content == false)
	{
//		__ipcam_display_content = true;
		var chn = streamtype;
		var str = "<object id=\"JaViewer_view\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"100%;\" width=\"100%\">";
		str += "<param name=\"movie\" value=\"JaViewer.swf\">";
		str += "<param name=\"allowFullScreen\" value=\"true\">";
		str += "<param name=\"allowScriptAccess\" value=\"always\">";
		str += "</object>";
		document.getElementById("ipcam_display").innerHTML = str;
		var player = document.getElementById("ipcam_display");
		player.innerHTML = str;
	}
	if(streamtype == 0){
		setTimeout("flash_video(\"JaViewer_view\", \"720p.264\")", 2000);
	}else if(streamtype == 1){
		setTimeout("flash_video(\"JaViewer_view\", \"360p.264\")", 2000);
	}else{
		setTimeout("flash_video(\"JaViewer_view\", \"720p.264\")", 2000);
	}
//	}
}

var DHiMPlayer,DHiMPlayer_image,DHiMPlayer_isp;
function connect()
{
	if(__ipcam_display_content == false)
	{
		__ipcam_display_content = true;
		var str = '<object id="DHiMPlayer" name="TestR" classid="clsid:A43D904E-1421-4c3d-B5F4-00CFE63EC63F" height = "1px;" width = "1px;" ></object>';
		document.getElementById("ipcam_display").innerHTML = str;
		DHiMPlayer = document.getElementById("DHiMPlayer");
		try{
			DHiMPlayer.attachEvent("InitInstance",function Init(ID,sInfo){});
	//		DHiMPlayer.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer.FullScreen(bFullScreen = !bFullScreen);});
			DHiMPlayer.attachEvent("ConRefuse",function OnConRefuse(res){
				var RefuseMsg = langstr.connect_refuse;
				alert(RefuseMsg);
			});	
		} catch (e) {

		}
	}
	
	if (DHiMPlayer.Version == "undefined")
	{
		load_attract();
	}
	else if(DHiMPlayer.Version >= "1, 0, 3, 0")
	{
		$('.mask_ipcam').css('display','none');
		$(DHiMPlayer).width('100%').height('100%');
		while(DHiMPlayer.ReadyFlag != true);
		DHiMPlayer.OpenByIP(g_ip,g_port);
		DHiMPlayer.UserLogon(g_usr,g_pwd);
		DHiMPlayer.AudioSwitch(true);
		DHiMPlayer.OpenChannel(0,0);
	}
	else
	{
		load_attract();
	}
}
function load_attract(){
	var btn = false;
/*    var timer = setInterval(function(){
			if(!btn){
				$("#manual_download").css("background-image","url(images/stream-change-hover.png)");
				$('.blink').css('color','#11E411');			
				btn = true;
			}else{
				$("#manual_download").css("background-image","url(images/stream-change.png)");
				$('.blink').css('color','#666');
				btn = false;
				}
		},500)*/
	
	}
function openChannel()
{
//	DHiMPlayer.OpenChannelEx(0);
}

function check_os_and_browser()
{
	var ua = navigator.userAgent;
	var ret = new Object();
    
	if (!!window.ActiveXObject || "ActiveXObject" in window){
		$('#manual_download_div')[0].style.display='block';
		ret.os = "windows";
        ret.browser = "ie";
        return ret;
	}

	if(/MSIE/i.test(ua) == true)
	{
		ret.os = "windows";
		if(/MSIE 9/i.test( ua ) == true)
		{
			ret.browser = "ie9";
		}
		else if(/MSIE 8/i.test( ua ) == true)
		{
			ret.browser = "ie8";
		}
		else if(/MSIE 7/i.test( ua ) == true)
		{
			ret.browser = "ie7";
		}
		else if(/MSIE 6/i.test( ua ) == true)
		{
			ret.browser = "ie6";
		}
		else
		{
			ret.browser = "ie";
			$('#manual_download_div')[0].style.display='block';
		}
	}
	else if(/Chrome/i.test( ua ) == true)
	{
		ret.browser = "chrome";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Android/i.test( ua ) == true)
		{
			ret.os = "android";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}	
	}
	else if(/Safari/i.test( ua ) == true)
	{
		ret.browser = "safari";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/iPhone/i.test( ua ) == true)
		{
			ret.os = "iphone";
		}
		else if(/iPad/i.test( ua ) == true)
		{
			ret.os = "ipad";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Android/i.test( ua ) == true)
		{
			ret.os = "android";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}
	}
	else if(/Firefox/i.test( ua ) == true)
	{
		ret.browser = "firefox";
		if(/Macintosh/i.test( ua ) == true)
		{
			ret.os = "mac";
		}
		else if(/Windows/i.test( ua ) == true)
		{
			ret.os = "windows";
		}
		else if(/Linux/i.test( ua ) == true)
		{
			ret.os = "linux";
		}
		else
		{
			ret.os = "";
		}
	}
	else
	{
		ret.browser = "";
		ret.os = "";
	}
	
//	if(ret.os == "" || ret.browser == "")
//	{
//		alert(ua);	
//	}
//	alert("os="+ret.os+",browser="+ret.browser);
	return ret;
}

function load1()
{
	g_port=document.location.port;

	if(g_port=="")	//80
	{ 
		g_port=80;
	}
/*	else
	{
		var i=document.location.host.indexOf(":");
		ip=document.location.host.substring(0,i);
	}
*/
/*
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		connect()
		openChannel();
	}//end ie
	else if(navigator.appName.indexOf("Netscape") != -1)//chrome,firefox
	{
		flv_view(0);
	}
*/
	var os_browser = check_os_and_browser();
	
//	set_wireless_page();
//	set_playback_page();
	if (!!window.ActiveXObject || "ActiveXObject" in window){
		$('.mask_ipcam').css('display','block');
		if(__ipcam_display_content == false)
		{
			__ipcam_display_content = true;
			var str = '<object id="DHiMPlayer" name="TestR" classid="clsid:A43D904E-1421-4c3d-B5F4-00CFE63EC63F" height = "1px;" width = "1px;" ></object>';
			document.getElementById("ipcam_display").innerHTML = str;
			DHiMPlayer = document.getElementById("DHiMPlayer");
			if(os_browser.browser == "ie6" || os_browser.browser == "ie7")
			{
				try{
					DHiMPlayer.attachEvent("InitInstance",function Init(ID,sInfo){});
		//				DHiMPlayer.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer.FullScreen(bFullScreen = !bFullScreen);});
					DHiMPlayer.attachEvent("ConRefuse",function OnConRefuse(res){
						var RefuseMsg = langstr.connect_refuse;
						alert(RefuseMsg);
					});	
				} catch (e){
				}
			//	openChannel();
			}
		}
		if (DHiMPlayer.Version == "undefined")
		{
			load_attract();
		}
		else if(DHiMPlayer.Version >= "1, 0, 3, 0")
		{
			$('.mask_ipcam').css('display','none');
			$(DHiMPlayer).width('100%').height('100%');
			while(DHiMPlayer.ReadyFlag != true);
			DHiMPlayer.OpenByIP(g_ip,g_port);
			DHiMPlayer.UserLogon(g_usr,g_pwd);
			DHiMPlayer.AudioSwitch(true);
			DHiMPlayer.OpenChannel(0,0);
		}
		else
		{
			load_attract();
		}
	}
	else if(os_browser.os == "iphone" || os_browser.os == "ipad")
	{
		flv_view(0);
	}
	else if(os_browser.os == "android")
	{
		flv_view(0);
	}
	else
	{
		flv_view(0);
	}
}

function streamchange(stream_id)
{
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		DHiMPlayer.CloseChannel();
		DHiMPlayer.OpenChannel(0, stream_id);
		// location.reload();
		// if(DHiMPlayer){
				// DHiMPlayer.OpenByIP(g_ip,g_port);
				// DHiMPlayer.UserLogon(g_usr,g_pwd);
				// DHiMPlayer.AudioSwitch(true);
				// DHiMPlayer.OpenChannel(0,stream_state);
			// }
	}
	else if (navigator.userAgent.indexOf("rv:11.0") != -1)
	{
		DHiMPlayer.CloseChannel();
		DHiMPlayer.OpenChannel(0, stream_id);
		
		// if(DHiMPlayer){
				// DHiMPlayer.OpenByIP(g_ip,g_port);
				// DHiMPlayer.UserLogon(g_usr,g_pwd);
				// DHiMPlayer.AudioSwitch(true);
				// DHiMPlayer.OpenChannel(0,stream_state);
			// }
	}
	else if (navigator.appName.indexOf("Netscape") != -1)
	{
		flv_view(stream_id);
	}
	else{
		alert("browser not support!");
	}
}
function audiochange(audiotype)
{
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		//DHiMPlayer.CloseChannelEx(streamtype?0:1);
		//DHiMPlayer.OpenChannelEx(streamtype);
		//location.reload();
		if(DHiMPlayer){
				DHiMPlayer.OpenByIP(g_ip,g_port);
				DHiMPlayer.UserLogon(g_usr,g_pwd);
				DHiMPlayer.AudioSwitch(Switch);
				//DHiMPlayer.OpenChannel(0,stream_state);
			}
	}
	else if (navigator.userAgent.indexOf("rv:11.0") != -1)
	{
			if(DHiMPlayer){
				DHiMPlayer.OpenByIP(g_ip,g_port);
				DHiMPlayer.UserLogon(g_usr,g_pwd);
				DHiMPlayer.AudioSwitch(Switch);
				//DHiMPlayer.OpenChannel(0,stream_state);
			}
	}
	else if (navigator.appName.indexOf("Netscape") != -1)
	{
		//flv_view(streamtype);
	}
	else{
		alert("browser not support!");
	}
}
function snap()
{
//	DHiMPlayer.Snapshot();
}
function records()
{
//	if(DHiMPlayer.GetLocalRecStatus(0)==true)
//	{
//		DHiMPlayer.EnableLocalRec(0,false);
//	}
//	else
//	{
//		DHiMPlayer.EnableLocalRec(0,true);
//	}
}
function playback()
{
//	DHiMPlayer.PlayBack();
}
function fullscreen_func()
{
	var os_browser = check_os_and_browser();
	if(os_browser.browser == "ie" || os_browser.browser == "ie6" || os_browser.browser == "ie7" || os_browser.browser == "ie8" || os_browser.browser == "ie9")
	{
		var DHiMPlayer=document.getElementById("Test");
		DHiMPlayer.fullscreen(bFullScreen = !bFullScreen);
	}
	else
	{
		//var obj = document.getElementById("JaViewer_view");
		var obj=$('#JaViewer_view');
		obj.fullStage();
	}
}

//消除object函数
function purify(obj)
{
}

//video function
function video_data2ui(dvr_data, sensor_type)
{
	var max_frate = 30;
	/*switch(dvr_data.juan.conf.vin0.shutter)
	{
	case "50hz":
		max_frate = 25;
		break;
	case "60hz":
	default:
		max_frate = 30;
		break;
	}*/
	
	for(var i = 0; i < $("#juan_envload\\#video\\@m_shutter")[0].options.length; i++)
	{
		if($("#juan_envload\\#video\\@m_shutter")[0].options[i].value == dvr_data.juan.conf.vin0.shutter)
		{
			$("#juan_envload\\#video\\@m_shutter")[0].selectedIndex = i;
		}
	}
	if(sensor_type == "ar0130"){
		$("#juan_envload\\#video\\@m_resol option").eq(2).html('1280x960');
	}else{
		$("#juan_envload\\#video\\@m_resol option").eq(2).html('1280x720');
	}
	reset_frame_rate_select($("#juan_envload\\#video\\@m_frate")[0], max_frate);
	reset_frame_rate_select($("#juan_envload\\#video\\@s_frate")[0], max_frate);
	if(g_model != "N18C"){
	reset_frame_rate_select($("#juan_envload\\#video\\@ss_frate")[0], max_frate);
	}

//	for(var i = $("#juan_envload\\#video\\@m_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@m_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@m_frate")[0].add(oOption);
//	}
//
//	for(var i = $("#juan_envload\\#video\\@s_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@s_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@s_frate")[0].add(oOption);
//	}
//
//	for(var i = $("#juan_envload\\#video\\@ss_frate")[0].options.length - 1; i >= 0; i--)
//	{
//		$("#juan_envload\\#video\\@ss_frate")[0].options.remove(i);
//	}
//	for(var i = 0; i < max_frate; i++)
//	{
//		var oOption = document.createElement("OPTION");
//		oOption.text = i + 1 + "fps";
//		oOption.value = i + 1;
//		$("#juan_envload\\#video\\@ss_frate")[0].add(oOption);
//	}	
	$("#juan_envload\\#video\\@m_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2640.stream0.bps;
	$("#juan_envload\\#video\\@s_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2641.stream0.bps;
	if(g_model != "N18C"){
		$("#juan_envload\\#video\\@ss_bitrate")[0].value = dvr_data.juan.conf.vin0.encode_h2642.stream0.bps;      //码流
	}
	if(max_frate >= dvr_data.juan.conf.vin0.encode_h2640.stream0.fps){
		$("#juan_envload\\#video\\@m_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2640.stream0.fps;
	}else{
		$("#juan_envload\\#video\\@m_frate")[0].value = max_frate;
	}
	if(max_frate >= dvr_data.juan.conf.vin0.encode_h2641.stream0.fps){
		$("#juan_envload\\#video\\@s_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2641.stream0.fps;
	}else{
		$("#juan_envload\\#video\\@s_frate")[0].value = max_frate;
		}
	if(g_model != "N18C"){
		if(max_frate >= dvr_data.juan.conf.vin0.encode_h2642.stream0.fps){
			$("#juan_envload\\#video\\@ss_frate")[0].value = dvr_data.juan.conf.vin0.encode_h2642.stream0.fps;		//帧率
		}else{
			$("#juan_envload\\#video\\@ss_frate")[0].value = max_frate;
		}
	}
	if(dvr_data.juan.conf.vin0.encode_h2640.stream0.mode == "cbr")											//码流可变
	{
		$("#juan_envload\\#video\\@m_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@m_brfmt1")[0].checked = true;
	}
	if(dvr_data.juan.conf.vin0.encode_h2641.stream0.mode == "cbr")
	{
		$("#juan_envload\\#video\\@s_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@s_brfmt1")[0].checked = true;
	}
	/*if(dvr_data.juan.conf.vin0.encode_h2642.stream0.mode == "cbr")
	{
		$("#juan_envload\\#video\\@ss_brfmt")[0].checked = true;
	}
	else
	{
		$("#juan_envload\\#video\\@ss_brfmt1")[0].checked = true;
	}*/
	m_bps_type();
	s_bps_type();
	ss_bps_type();
	switch (dvr_data.juan.conf.vin0.encode_h2640.stream0.size)												//分辨犿	
	{
		case "180p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 1;break;
		case "720p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 2;break;
		case "1080p": $("#juan_envload\\#video\\@m_resol")[0].selectedIndex = 3;break;
		default: break;
	}
	switch (dvr_data.juan.conf.vin0.encode_h2641.stream0.size)																						
	{
		case "180p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 1;break;
		//case "720p": $("#juan_envload\\#video\\@s_resol")[0].selectedIndex = 2;break;
		default: break;
	}
	/*switch (dvr_data.juan.conf.vin0.encode_h2642.stream0.size)																						
	{
		case "180p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 0;break;
		case "360p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 1;break;
		case "720p": $("#juan_envload\\#video\\@ss_resol")[0].selectedIndex = 2;break;
		default: break;
	}*/

	switch (dvr_data.juan.conf.vin0.encode_h2640.stream0.quality)											//编码质量
	{
		case "highest": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 0;break;
		case "high": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 1;break;
		case "medium": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 2;break;   
		case "low": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 3;break;
		case "lowest": $("#juan_envload\\#video\\@m_qual")[0].selectedIndex = 4;break;
		default: break;
	}
	switch (dvr_data.juan.conf.vin0.encode_h2641.stream0.quality)
	{
		case "highest": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 0;break;
		case "high": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 1;break;
		case "medium": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 2;break;
		case "low": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 3;break;
		case "lowest": $("#juan_envload\\#video\\@s_qual")[0].selectedIndex = 4;break;
		default: break;
	}	
	if(dvr_data.juan.conf.vin0.encode_h2642)
	{
		switch (dvr_data.juan.conf.vin0.encode_h2642.stream0.quality)
		{
			case "highest": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 0;break;
			case "high": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 1;break;
			case "medium": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 2;break;
			case "low": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 3;break;
			case "lowest": $("#juan_envload\\#video\\@ss_qual")[0].selectedIndex = 4;break;
			default: break;
		}
	}
}
function video_preload_content()
{
	NetSDK_User(g_usr, g_pwd);
	NetSDK_GetJSON("/NetSDK/System/deviceInfo", function(Obj){
		$("#juan\\#devinfo\\@name").val(Obj.deviceName);
		$("#juan\\#devinfo\\@model").val(Obj.model);
		$("#juan\\#devinfo\\@swver").val(Obj.firmwareVersion);
		$("#juan\\#devinfo\\@sdk").val(Obj.sdkVersion);
		if (null != Obj.firmwareReleaseDate) {
			var idx = String(Obj.firmwareReleaseDate).indexOf("T");
			var str = String(Obj.firmwareReleaseDate);
			if (idx >= 0) {
				$("#juan\\#devinfo\\@reldatetime").val(str.substring(0, idx));
			}
		}
	});

}

function bitrate_change(id,min_,max_)
{
	var inbox=document.getElementById(id);
	var str=inbox.value;
	if(isNaN(str)==true)
	{
		alert("Not a Number,Please retry");
		inbox.value=max_;
	}
	else{
		if(Number(str)>max_)
		{
			alert("Input number is too big,Please retry");
			inbox.value=max_;
		}
		else if(Number(str)<min_)
		{
			alert("Input number is too small,Please retry");
			inbox.value=min_;
		}
	}
}


//image
function image_data2ui(data)
{
	var con,bri,sat,hue,flip,mirror, sharpness;
	var num = Array(4), maxmum = Array(4);
	con = data.contrastLevel;
	bri = data.brightnessLevel;
	sat = data.saturationLevel;
	hue = data.hueLevel;
	flip = data.flipEnabled;
	mirror = data.mirrorEnabled;
	if(flip == true){
		$('#Flip').prop('checked',true);
	}else{
		$('#Flip').prop('checked',false);
	}
	if(mirror == true){
		$('#Mirror').prop('checked',true);
	}else{
		$('#Mirror').prop('checked',false);
	}
	num[0] = parseInt(con); maxmum[0] = parseInt(100);
	num[1] = parseInt(bri); maxmum[1] = parseInt(100);
	num[2] = parseInt(sat); maxmum[2] = parseInt(100);
	num[3] = parseInt(hue); maxmum[3] = parseInt(100);
	slider_con.n_maxValue = maxmum[0];
	slider_con.n_pix2value = slider_con.n_pathLength / (slider_con.n_maxValue - slider_con.n_minValue);
	slider_lum.n_maxValue = maxmum[1];
	slider_lum.n_pix2value = slider_lum.n_pathLength / (slider_lum.n_maxValue - slider_lum.n_minValue);
	slider_sat.n_maxValue = maxmum[2];
	slider_sat.n_pix2value = slider_sat.n_pathLength / (slider_sat.n_maxValue - slider_sat.n_minValue);
	slider_hue.n_maxValue = maxmum[3];
	slider_hue.n_pix2value = slider_hue.n_pathLength / (slider_hue.n_maxValue - slider_hue.n_minValue);
	/*$('#juan_envload#color@con')[0].value = num[0];*/
	$("#juan_envload\\#color\\@con")[0].value = num[0];
	$("#juan_envload\\#color\\@lum")[0].value = num[1];
	$("#juan_envload\\#color\\@sat")[0].value = num[2];
	$("#juan_envload\\#color\\@hue")[0].value = num[3];
	slider_con.f_setValue(num[0],0);
	slider_lum.f_setValue(num[1],0);
	slider_sat.f_setValue(num[2],0);
	slider_hue.f_setValue(num[3],0);
	isp_load_sharpness();
	$("#juan_envload\\#color\\@con").prop('disabled', true);
	$("#juan_envload\\#color\\@lum").prop('disabled',true);
	$("#juan_envload\\#color\\@sat").prop('disabled',true);
	$("#juan_envload\\#color\\@hue").prop('disabled',true);
	$('#isp\\@sharpness').prop('disabled',true);
}

// var __display_board_content = false;
// function showpreview()
// {
// 	if(g_port=="")	//80
// 	{
// 		g_port=80;
// 	}
// /*	else
// 	{
// 		var i=document.location.host.indexOf(":");
// 		ip=document.location.host.substring(0,i);
// 	}
// */
// 	var os_browser = check_os_and_browser();
// 	if(os_browser.browser == "ie" || os_browser.browser == "ie6" || os_browser.browser == "ie7" || os_browser.browser == "ie8" || os_browser.browser == "ie9")
// 	{
// 		if(__display_board_content == false)
// 		{
// 			__display_board_content = true;	
// 			var td1 = document.getElementById('preview_board');
// 			var str = '<object id="DHiMPlayer_image" name="TestR" classid="clsid:A43D904E-1421-4c3d-B5F4-00CFE63EC63F" height = "380" width = "100%"></object>';
// 			document.getElementById('display_board').innerHTML = str;
// 			DHiMPlayer_image = document.getElementById("DHiMPlayer_image");
// 			try{
// 				DHiMPlayer_image.attachEvent("InitInstance",function Init(ID,sInfo){});
// 				DHiMPlayer_image.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer_image.FullScreen(bFullScreen = !bFullScreen);});
// 			}catch (e){
// 			}
// 		}
// 		if(DHiMPlayer_isp){
// 			DHiMPlayer_isp.close();
// 		}

// 		if(DHiMPlayer_image.Version >= "1, 0, 2, 0")
// 		{
// 			while(DHiMPlayer_image.ReadyFlag != true);
// 			DHiMPlayer_image.OpenByIP(g_ip,g_port);
// 			DHiMPlayer_image.UserLogon(g_usr,g_pwd);
// 			DHiMPlayer_image.OpenChannel(0,1);
// 		}
// 	}
// 	else
// 	{

// 			var td1 = document.getElementById('preview_board');
// 			var str = "<object id=\"JaViewer_preview\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"447px;\" width=\"100%\">";
// 			str += "<param name=\"movie\" value=\"JaViewer.swf\">";
// 			str += "<param name=\"allowFullScreen\" value=\"true\">";
// 			str += "<param name=\"allowScriptAccess\" value=\"always\">";
// 			str += "</object>";
// 			var player = document.getElementById("display_board");
// 			player.innerHTML = str;
	
// 	}
// 		setTimeout("flash_video(\"JaViewer_preview\", \"360p.264\")", 1000);

// }
function image_load_content()
{
	// showpreview();
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/video/input/channel/1?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
				image_data2ui(data);
			}
		});
}

function isp_load_sharpness()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/image?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			},
			success:function(data){	
				$('#manualSharpness').val(data.manualSharpness.enabled);
				$('#isp\\@sharpness').val(data.manualSharpness.sharpnessLevel);
				slider_sha.n_maxValue = 255;
				slider_sha.n_pix2value = slider_sha.n_pathLength / (slider_sha.n_maxValue - slider_sha.n_minValue);			
				slider_sha.f_setValue(data.manualSharpness.sharpnessLevel ,0);
				$('#manualSharpness').prop('checked',data.manualSharpness.enabled);
				/*$('#manualSharpness')[0].onclick = function(){
					var obj1 = $('#isp\\@sharpness');
					obj1.prop('disabled',!obj1.prop('disable'));
					slider_sha.prop('disabled', false);
				}*/
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
		});
}
function image_default(){
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/video/input/channel/1/properties?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
				 $("#juan_envload\\#color\\@lum")[0].value=data.brightnessLevelProperty.def;
				 slider_lum.f_setValue(parseInt(data.brightnessLevelProperty.def),0);
				$("#juan_envload\\#color\\@con")[0].value= data.contrastLevelProperty.def;
				slider_con.f_setValue(parseInt(data.contrastLevelProperty.def),0);
				$("#isp\\@sharpness")[0].value= data.sharpnessLevelProperty.def;
				slider_sha.f_setValue(parseInt(data.sharpnessLevelProperty.def),0);
				$("#juan_envload\\#color\\@sat")[0].value= data.saturationLevelProperty.def;
				slider_sat.f_setValue(parseInt(data.saturationLevelProperty.def),0);
				$("#juan_envload\\#color\\@hue")[0].value= data.hueLevelProperty.def;
				slider_hue.f_setValue(parseInt(data.hueLevelProperty.def),0);
				image_save_content();
			}
		});
}
function image_save_content()
{
	showInfo(langstr.save_setup);
	var con = $("#juan_envload\\#color\\@con")[0].value,
		lum = $("#juan_envload\\#color\\@lum")[0].value,
		sat = $("#juan_envload\\#color\\@sat")[0].value;
		hue = $("#juan_envload\\#color\\@hue")[0].value;
		flip = $('#Flip').prop('checked') ?true:false;
		mirror = $('#Mirror').prop('checked') ?true:false; 
	var sharpnessLevel = $('#isp\\@sharpness')[0].value;
	var manualSharpnessEnabled = $('#manualSharpness').prop('checked') ?true:false;;

	var Json = new Object;
	Json.hueLevel = parseInt(hue, 0);
	Json.contrastLevel = parseInt(con, 0);
	Json.saturationLevel = parseInt(sat, 0);
	Json.brightnessLevel = parseInt(lum, 0);
	Json.flipEnabled = flip;
	Json.mirrorEnabled = mirror;
	
	var data = $.toJSON(Json);
	//var data = '{ "hueLevel": '+hue+', "contrastLevel": '+con+', "saturationLevel": '+sat+', "brightnessLevel": '+lum+', "flipEnabled": '+flip+', "mirrorEnabled": '+mirror+' }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/video/input/channel/1?r=' + Math.random(),
			dataType:'json',
			data:data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
				if(data.statusCode == 0){ 
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
					//alert('statusCode='+data.statusCode);
				}
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		})
	var Json = new Object;
	Json.enabled = manualSharpnessEnabled;
	Json.sharpnessLevel = sharpnessLevel;
	
	var isp_data = $.toJSON(Json);
	//var isp_data = '{ "enabled": '+manualSharpnessEnabled+', "sharpnessLevel": '+sharpnessLevel+' }';
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/image/manualSharpness?r=' + Math.random(),
			dataType:'json',
			data:isp_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
			},
			error:function(a,b,c){ 
			}
		})
}
function image_preview_content(id)
{
	showInfo(langstr.update_preview);
	var data;
	var is_isp = false;
if(id == 'sl0slider' ){
	var lum = $("#juan_envload\\#color\\@lum")[0].value;
	data = '{"brightnessLevel": '+lum+'}';
}else if(id == 'sl1slider'){
	var con = $("#juan_envload\\#color\\@con")[0].value;
	data = '{"contrastLevel": '+con+'}';
}else if(id == 'sl2slider'){
	var sat = $("#juan_envload\\#color\\@sat")[0].value;
	data = '{"saturationLevel": '+sat+'}';
 }else if(id == 'sl3slider'){
	var hue = $("#juan_envload\\#color\\@hue")[0].value;
	data = '{ "hueLevel": '+hue+' }';
}else if(id == 'sl4slider'){
	var manual = $('#manualSharpness').prop('checked') ?'true':'false';
	var sharpness = $("#isp\\@sharpness")[0].value;
	is_isp = true;
	data = '{"manualSharpness" :{"enabled" : '+ manual + ',"sharpnessLevel" : ' + sharpness + '}}';
}else if(id == 'sl5slider' ){
	var strength = $("#isp\\@wdr_str")[0].value;
	is_isp = true;
	data = '{"WDR" :{"WDRStrength" : ' + strength + '}}';
}else if(id == 'sl6slider' ){
	var strength = $("#isp\\@denoise_strength")[0].value;
	is_isp = true;
	data = '{"denoise3d" :{"denoise3dStrength" : ' + strength + '}}';
}else if(id == 'Flip'){
	var flip = $('#Flip').prop('checked') ?'true':'false';
	data = '{"flipEnabled": '+flip+'}';
}else if(id == 'Mirror'){
	var mirror = $('#Mirror').prop('checked') ?'true':'false'; 
	data = '{ "mirrorEnabled": '+mirror+' }';
}else{
	/*var xmlstr ='';
	xmlstr += 'error';
	alert(xmlstr);*/
}
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	if(is_isp){
		var ipc_url = dvr_url + '/netsdk/image?r=' + Math.random();
	}else{
		var ipc_url = dvr_url + '/netsdk/video/input/channel/1?r=' + Math.random();
	}
	$.ajax({
			type:'PUT',
			url:ipc_url,
			dataType:'json',
			data:data,
			async:false,
			beforeSend : function(req ) {
	      	req .setRequestHeader('Authorization', auth);
	  		},
			success:function(data){ 
				if(data.statusCode == 0){ 
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
					//alert('statusCode='+data.statusCode);
				}
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});
}


//audio
function audio_data2ui(dvr_data)
{
    $("#audio\\@inputMode").empty();
    for(i=0;i<dvr_data.microphoneTypeProperty.opt.length;i++)
    {
        switch(dvr_data.microphoneTypeProperty.opt[i]){
            case "auto": $("#audio\\@inputMode").append('<option value="auto">'+langstr.audio_input_auto+'</option>');break;
            case "activePickup": $("#audio\\@inputMode").append('<option value="activePickup">'+langstr.audio_input_line+'</option>');break;
            case "passiveMic": $("#audio\\@inputMode").append('<option value="passiveMic">'+langstr.audio_input_mic+'</option>');break;
            default:break;
        }
        
    }
    switch(dvr_data.microphoneType)
	{
		//case "auto": $("#audio\\@inputMode")[0].selectedIndex = 0;break;
        case "activePickup": $("#audio\\@inputMode")[0].selectedIndex = 0;break;
		case "passiveMic": $("#audio\\@inputMode")[0].selectedIndex = 1;break;
		default:break;
	}
}

//isp
function isp_data2ui(dvr_data)
{
//	$("#isp\\@wdr_str")[0].value = dvr_data.WDR.WDRStrength;
//	$("#isp\\@denoise_strength")[0].value = dvr_data.denoise3d.denoise3dStrength;
//	slider_str.f_setValue(dvr_data.WDR.WDRStrength,0);
//	slider_denoise_strength.f_setValue(dvr_data.denoise3d.denoise3dStrength,0);
//	slider_str.n_pix2value = slider_str.n_pathLength / (slider_str.n_maxValue - slider_str.n_minValue);
//	slider_denoise_strength.n_pix2value = slider_denoise_strength.n_pathLength / (slider_denoise_strength.n_maxValue - slider_denoise_strength.n_minValue);
//	$("#isp\\@wdr_str").prop('disabled', true);
//	$("#isp\\@denoise_strength").prop('disabled', true);
/*	switch(dvr_data.sceneMode)
	{
		case "auto": $("#isp\\@scene_mode")[0].selectedIndex = 0;break;
		case "indoor": $("#isp\\@scene_mode")[0].selectedIndex = 1;break;
		case "outdoor": $("#isp\\@scene_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.awbMode)
	{
		case "auto": $("#isp\\@white_balance_mode")[0].selectedIndex = 0;break;
		case "indoor": $("#isp\\@white_balance_mode")[0].selectedIndex = 1;break;
		case "outdoor": $("#isp\\@white_balance_mode")[0].selectedIndex = 2;break;
		default:break;
	}*/
	switch(dvr_data.irCutFilter.irCutControlMode)
	{
		case "hardware": $("#isp\\@ircut_control_mode")[0].selectedIndex = 0;break;
		case "software": $("#isp\\@ircut_control_mode")[0].selectedIndex = 1;break;
		default:break;
	}
	switch(dvr_data.irCutFilter.irCutMode)
	{
		case "auto": $("\#isp\\@ircut_mode")[0].selectedIndex = 0;break;
		case "daylight": $("#isp\\@ircut_mode")[0].selectedIndex = 1;break;
		case "night": $("#isp\\@ircut_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.imageStyle)
	{
		case 1: $("#isp\\@imageStyle")[0].selectedIndex = 0;break;
		case 2: $("#isp\\@imageStyle")[0].selectedIndex = 1;break;
		case 3: $("#isp\\@imageStyle")[0].selectedIndex = 2;break;
		default:break;
	}
/*	if(dvr_data.WDR.enabled)
	{
		 $("#isp\\@wide_dynamic_range_enable")[0].selectedIndex = 0;		
	}else{
		$("#isp\\@wide_dynamic_range_enable")[0].selectedIndex = 1;
	}*/
	/*switch(dvr_data.juan.conf.isp.exposure.mode)
	{
		case "auto": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 0;break;
		case "bright": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 1;break;
		case "dark": $("#juan_envload\\#isp\\@exposure_mode")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.denoise.denoise_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex = 1;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.advance.anti_fog_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex = 1;break;
		default:break;
	}*/
/*	switch(dvr_data.lowlightMode)
	{
		case "close": $("#isp\\@lowlight_enable")[0].selectedIndex = 0;break;
		case "only night": $("#isp\\@lowlight_enable")[0].selectedIndex = 1;break;
		case "day-night": $("#isp\\@lowlight_enable")[0].selectedIndex = 2;break;
		case "auto": $("#isp\\@lowlight_enable")[0].selectedIndex = 3;break;
		default:break;
	}*/
	/*switch(dvr_data.juan.conf.isp.advance.gamma)
	{
		case "default": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 0;break;
		case "normal": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 1;break;
		case "high": $("#juan_envload\\#isp\\@gamma")[0].selectedIndex = 2;break;
		default:break;
	}
	switch(dvr_data.juan.conf.isp.advance.defect_pixel_enable)
	{
		case "yes": $("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex = 0;break;
		case "no": $("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex = 1;break;
		default:break;
	}*/
}

// var __display_board1_content = false;
// function showpreview_isp()
// {
// 	if(g_port=="")	//80
// 	{
// 		g_port=80;
// 	}
// //	else
// //	{
// //		var i=document.location.host.indexOf(":");
// //		ip=document.location.host.substring(0,i);
// //	}

// 	var os_browser3 = check_os_and_browser();
// 	if(os_browser3.browser == "ie" || os_browser3.browser == "ie6" || os_browser3.browser == "ie7" || os_browser3.browser == "ie8" || os_browser3.browser == "ie9")
// 	{
// 		if(__display_board1_content == false)
// 		{
// 			__display_board1_content = true;	
// 		var str1 = '<object id="DHiMPlayer_isp" name="TestR" classid="clsid:A43D904E-1421-4c3d-B5F4-00CFE63EC63F" height = "380" width = "100%"></object>';
// 		document.getElementById('display_board').innerHTML = str1;
// 		DHiMPlayer_isp = document.getElementById("DHiMPlayer_isp");
// 		try{
// 			DHiMPlayer_isp.attachEvent("InitInstance",function Init(ID,sInfo){});
// 			DHiMPlayer_isp.attachEvent("LDBClick",function LClick(x,y){DHiMPlayer_isp.FullScreen(bFullScreen = !bFullScreen);});
// 		}catch (e){
// 		}
// 	}
// 		if(DHiMPlayer_image){
// 			DHiMPlayer_image.close();
// 		}
// 				while(DHiMPlayer_isp.ReadyFlag != true);
// 				DHiMPlayer_isp.OpenByIP(g_ip,g_port);
// 				DHiMPlayer_isp.UserLogon(g_usr,g_pwd);
// 				DHiMPlayer_isp.OpenChannel(0,1);
// 	}
// 	else
// 	{
// //		if(__display_board_content == false)
// //		{
// //			__display_board_content = true;
// 			var td2 = document.getElementById('preview_board1');
// 			var str1 = "<object id=\"JaViewer_preview1\" type=\"application/x-shockwave-flash\" data=\"JaViewer.swf\" style=\"outline: medium none;\" height=\"447px;\" width=\"100%\">";
// 			str1 += "<param name=\"movie\" value=\"JaViewer.swf\">";
// 			str1 += "<param name=\"allowFullScreen\" value=\"true\">";
// 			str1 += "<param name=\"allowScriptAccess\" value=\"always\">";
// 			str1 += "</object>";
// 			var player = document.getElementById("display_board1");
// 			player.innerHTML = str1;
	
// 	}
// 		setTimeout("flash_video(\"JaViewer_preview1\", \"360p.264\")", 1000);

// }
function audio_load_content()
{	
//	alert(xmlstr);
	// showpreview_isp();
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
		$.ajax({
				type:"GET",
				url:dvr_url + '/NetSDK/Audio/input/channel/1/properties?r=' + Math.random(),
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
					audio_data2ui(data);	
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
			});
}

function audio_save_content()
{
	showInfo(langstr.save_setup);
	var audio_input_mode, audio_data;
 
	switch($("#audio\\@inputMode")[0].selectedIndex)
	{
		//case 0: audio_input_mode = "auto";break;
        case 0: audio_input_mode = "activePickup";break;
		case 1: audio_input_mode = "passiveMic";break;
		default:break;
	}
 
    var Json = new Object;
	Json.microphoneType = audio_input_mode;
	var audio_data = $.toJSON(Json);

	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:'PUT',
			url:dvr_url + '/NetSDK/Audio/input/channel/1?r=' + Math.random(),
			dataType:'json',
			data:audio_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
				if(data.statusCode == 0){ 
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
				}
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});
}

function audio_preview_content(id,value)
{           
	showInfo(langstr.update_preview);
	var audio_data;
    if(id == 'audio@inputMode' ){
	audio_data = '{"microphoneType": "'+ value +'"}';
    }
	//var data = '{ "hueLevel": '+hue+', "contrastLevel": '+con+', "saturationLevel": '+sat+', "brightnessLevel": '+lum+', "flipEnabled": '+flip+', "mirrorEnabled": '+mirror+' }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:'PUT',
			url:dvr_url + '/NetSDK/Audio/input/channel/1?r=' + Math.random(),
			dataType:'json',
			data:audio_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
		success: function(data, textStatus){
			showInfo(langstr.save_preview);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}

function isp_load_content()
{	
//	alert(xmlstr);
	// showpreview_isp();
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
		$.ajax({
				type:"GET",
				url:dvr_url + '/netsdk/image?r=' + Math.random(),
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
					isp_data2ui(data);	
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
			});
}
function isp_save_content()
{
	showInfo(langstr.save_setup);
//	var wdr_str = $("#isp\\@wdr_str")[0].value;
//	var denoise3dStrength = $("#isp\\@denoise_strength")[0].value;
	var wdr_str;
	var denoise3dStrength;
	var scene_mode,white_balance_mode,ircut_control_mode,ircut_mode,wide_dynamic_range_enable,exposure_mode,denoise_enable,anti_fog_enable,lowlight_enable,gamma,defect_pixel_enable,image_style;
/*	switch($("#isp\\@scene_mode")[0].selectedIndex)
	{
		case 0: scene_mode = "auto";break;
		case 1: scene_mode = "indoor";break;
		case 2: scene_mode = "outdoor";break;
		default:break;
	}*/
/*	switch($("#isp\\@white_balance_mode")[0].selectedIndex)
	{
		case 0: white_balance_mode = "auto";break;
		case 1: white_balance_mode = "indoor";break;
		case 2: white_balance_mode = "outdoor";break;
		default:break;
	}*/
/*	switch($("#isp\\@imageStyle")[0].selectedIndex)
	{
		case 0: image_style = 1;break;
		case 1: image_style = 2;break;
		case 2: image_style = 3;break;
		default:break;
	}*/
	switch($("#isp\\@ircut_control_mode")[0].selectedIndex)
	{
		case 0: ircut_control_mode = "hardware";break;
		case 1: ircut_control_mode = "software";break;
		default:break;
	}
	switch($("#isp\\@ircut_mode")[0].selectedIndex)
	{
		case 0: ircut_mode = "auto";break;
		case 1: ircut_mode = "daylight";break;
		case 2: ircut_mode = "night";break;
		default:break;
	}
/*	switch($("#isp\\@wide_dynamic_range_enable")[0].selectedIndex)
	{
		case 0: wide_dynamic_range_enable = true;break;
		case 1: wide_dynamic_range_enable = false;break;
		default:break;
	}*/
	/*switch($("#isp\\@exposure_mode")[0].selectedIndex)
	{
		case 0: exposure_mode = "auto";break;
		case 1: exposure_mode = "bright";break;
		case 2: exposure_mode = "dark";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@denoise_enable")[0].selectedIndex)
	{
		case 0: denoise_enable = "yes";break;
		case 1: denoise_enable = "no";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@anti_fog_enable")[0].selectedIndex)
	{
		case 0: anti_fog_enable = "yes";break;
		case 1: anti_fog_enable = "no";break;
		default:break;
	}*/
/*	switch($("#isp\\@lowlight_enable")[0].selectedIndex)
	{
		case 0: lowlight_enable = "close";break;
		case 1: lowlight_enable = "only night";break;
		case 2: lowlight_enable = "day-night";break;
		case 3: lowlight_enable = "auto";break;
		default:break;
	}*/
	/*switch($("#juan_envload\\#isp\\@gamma")[0].selectedIndex)
	{
		case 0: gamma = "default";break;
		case 1: gamma = "normal";break;
		case 2: gamma = "high";break;
		default:break;
	}
	switch($("#juan_envload\\#isp\\@defect_pixel_enable")[0].selectedIndex)
	{
		case 0: defect_pixel_enable = "yes";break;
		case 1: defect_pixel_enable = "no";break;
		default:break;
	}*/

//	var isp_data = '{"sceneMode": "'+ scene_mode +'","imageStyle":"'+image_style+'","awbMode":"'+white_balance_mode+'","lowlightMode":"'+lowlight_enable+'","irCutFilter" : {"irCutControlMode" : "' + ircut_control_mode +'","irCutMode" : "' + ircut_mode +'",},"denoise3d" : {"denoise3dStrength" : ' + denoise3dStrength +',},"WDR" : {"enabled" : ' + wide_dynamic_range_enable +',"WDRStrength" : ' + wdr_str +',},}'
	var Json = new Object;
	Json.irCutFilter = new Object;
	Json.irCutFilter.irCutControlMode= ircut_control_mode;
	Json.irCutFilter.irCutMode = ircut_mode;
	var isp_data = $.toJSON(Json);
	//var isp_data = '{"irCutFilter" : {"irCutControlMode" : "' + ircut_control_mode +'","irCutMode" : "' + ircut_mode +'"}}';

	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/image?r=' + Math.random(),
			dataType:'json',
			data:isp_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
				if(data.statusCode == 0){ 
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
				}
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});
}
function isp_preview_content(id,value)
{
	showInfo(langstr.update_preview);
	var isp_data;
if(id == 'isp@scene_mode' ){
	isp_data = '{"sceneMode": "'+ value +'"}';
}else if(id == 'isp@white_balance_mode' ){
	isp_data = '{"awbMode": "' + value + '"}';
}else if(id == 'isp@ircut_control_mode' ){
	isp_data = '{"irCutFilter" : {"irCutControlMode" : "' + value +'"}}';
}else if(id == 'isp@ircut_mode' ){
	isp_data = '{"irCutFilter" : {"irCutMode" : "' + value +'"}}';
}else if(id == 'sl5slider' ){

}else if(id == 'isp@wide_dynamic_range_enable' ){
	var bool_value;
	if(value == "yes"){
		bool_value = true;
	}else{
		bool_value = false;
	}
	isp_data = '{"WDR" : {"enabled" : ' + bool_value +'}}';
}else if(id == 'sl6slider' ){

}else if(id == 'sl7slider' ){

}else if(id == 'isp@anti_fog_enable' ){

}else if(id == 'isp@lowlight_enable' ){
	isp_data = '{"lowlightMode": "'+ value +'"}';
}else if(id == 'isp@defect_pixel_enable' ){

}else if(id == 'isp@imageStyle' ){
	isp_data = '{"imageStyle": '+ value +'}';
}else{

	}
	//var data = '{ "hueLevel": '+hue+', "contrastLevel": '+con+', "saturationLevel": '+sat+', "brightnessLevel": '+lum+', "flipEnabled": '+flip+', "mirrorEnabled": '+mirror+' }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/image?r=' + Math.random(),
			dataType:'json',
			data:isp_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
		success: function(data, textStatus){
			showInfo(langstr.save_preview);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}
//encode
function check(){
	/*
	for(i=0;i<document.getElementsByName("channelName")[0].value.length;i++){
		var c = document.getElementsByName("channelName")[0].value.substr(i,1);
		var ts = escape(c);
		if(ts.substring(0,2) == "%u"){
		alert(langstr.can_not_input_Chinese);
		document.getElementsByName("channelName")[0].value = "";
		document.getElementsByName("channelName")[0].focus();
		}
	}
	*/
}
function incoming_load_fbase()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/video/input/channel/1?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			},
			success:function(data){	
				$('#encode_Fbase').val(data.powerLineFrequencyMode);
				encode_load();
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
		});
}

function audio_load()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/audio/encode/channel/101?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			},
			success:function(data){	
				$('#encodeType').val(data.enabled);
				if(data.enabled)
				{
					 $("#encodeType")[0].selectedIndex = 1;		
				}else{
					$("#encodeType")[0].selectedIndex = 0;
				}
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
		});
}

function set_wireless_page()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/network/wireless/status?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data, textStatus, context){
				if(data == false){
					del_li("ul_network", 3);	
				}
			}
		});
}

function clear_playback_button()
{
	$('#div_playback').remove();
}


function set_playback_page()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/sdcard/status?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data, textStatus, context){
				if(data != "work"){
					del_li("ul_advance", 3);
					clear_playback_button();	
				}
			}
		});
}

function deviceName_load()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	var id =$('#encode_id').val();
		$.ajax({
				type:"GET",
				url:dvr_url + '/netsdk/system/deviceinfo?r=' + Math.random(),
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){	
					$('.overlay_channelName').val(data.deviceName);
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
			});
}
function encode_load_content(model, sensor_type)
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	var id =$('#encode_id').val();
		$.ajax({
				type:"GET",
				url:dvr_url + '/netsdk/video/encode/channel/'+id+'/properties?r=' + Math.random(),
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
					//a = data.constantBitRateProperty.min;
//					b = data.constantBitRateProperty.max;
//					c = data.frameRateProperty.min;
//					d = data.frameRateProperty.max;
					$('#overlay_resolution').find('option').remove();
					for(i=0;i<data.resolutionProperty.opt.length;i++)
					{
						$("#overlay_resolution").append('<option>'+data.resolutionProperty.opt[i]+'</option>');
					}
					audio_load();
					incoming_load_fbase();
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
			});
}
function encode_load()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	var id =$('#encode_id').val();
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/video/encode/channel/'+ id+'?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
				free_resolution = data.freeResolution;
				encode_data2ui();		
				$('.overlay_channelName').val(data.channelName);		
				$('#overlay_resolution').val(data.resolution);
				$('#overlay_resolutionWidth').val(data.resolutionWidth);
				$('#overlay_resolutionHeight').val(data.resolutionHeight);
				$('#overlay_bitRateControlType').val(data.bitRateControlType);
				$('#overlay_constantBitRate').val(data.constantBitRate);
				$('#overlay_frameRate').val(data.frameRate);
				$('#h264_profile').val(data.h264Profile);
				$('#codecType').val(data.codecType);
				//$('#constantBitRate_td').html(langstr.frameRate + '(' + a + '~' + b + ')');
//				$('#frameRate_td').html(langstr.frameRate + '(' + c + '~' + d + ')');
				/*$('#overlay_freeresolution')[0].onclick = function(){
					var obj = $('#overlay_freeresolution_0');
					var obj1 = $('#overlay_freeresolution_1');
						obj.is(':visible') ? obj.hide() : obj.show();
						obj1.is(':visible') ? obj1.hide() : obj1.show();
				}*/
			}
		});
}
function encode_data2ui()
{
		if(free_resolution == true){
			$('#overlay_freeresolution').prop('checked',true);
			$('#overlay_freeresolution_0')[0].style.display='none';
			$('#overlay_freeresolution_1')[0].style.display='block';
		}else{
			$('#overlay_freeresolution').prop('checked',false);
			$('#overlay_freeresolution_0')[0].style.display='block';
			$('#overlay_freeresolution_1')[0].style.display='none';	
		}
		switch($('#id_freeresolution_1').val())
		{
			case 'true': $('#id_freeresolution_1')[0].checked = 1; break;
			case 'false': $('#id_freeresolution_0')[0].checked = 1; break;
			default: break;
		}
}
function encode_save_content()
{	
	showInfo(langstr.save_setup);
	var id =$('#encode_id').val();
	var overlay_channelName = $('.overlay_channelName').val();
	var overlay_resolution =$('#overlay_resolution :selected').html();
	var overlay_bitRateControlType =$('#overlay_bitRateControlType :selected').val();
	var overlay_freeresolution;
//	var overlay_resolutionWidth = $('#overlay_resolutionWidth').val();
//	var overlay_resolutionHeight = $('#overlay_resolutionHeight').val();
	var overlay_constantBitRate = $('#overlay_constantBitRate').val();
	var overlay_frameRate = $('#overlay_frameRate').val();
	var overlay_profile = $('#h264_profile :selected').val();
	var codecType = $('#codecType :selected').val();
	if($('input#overlay_freeresolution:checked').val() == 'on')
	{
		overlay_freeresolution = true;
	}else
	{
		overlay_freeresolution = false;
	}
//	var encode_data = '{"channelName": "'+ overlay_channelName+'","h264Profile":"'+overlay_profile+'","resolution":"'+overlay_resolution+'","freeResolution":'+overlay_freeresolution+',"resolutionWidth":'+ overlay_resolutionWidth +',"resolutionHeight":'+ overlay_resolutionHeight +',"bitRateControlType":"'+overlay_bitRateControlType+'","constantBitRate":'+ overlay_constantBitRate +',"frameRate":'+ overlay_frameRate +'}';


	var Json = new Object;
	Json.thanks = 'ありがとう';
	Json.channelName = overlay_channelName;
	Json.h264Profile = overlay_profile;
	Json.resolution = overlay_resolution;
	Json.bitRateControlType = overlay_bitRateControlType;
	Json.constantBitRate = parseInt(overlay_constantBitRate);
	Json.frameRate = parseInt(overlay_frameRate);
	Json.codecType = codecType;

	var encode_data = $.toJSON(Json)
    audio_save();

	//var encode_data = '{"channelName": "'+ overlay_channelName+'","h264Profile":"'+overlay_profile+'","resolution":"'+overlay_resolution+'","bitRateControlType":"'+overlay_bitRateControlType+'","constantBitRate":'+ overlay_constantBitRate +',"frameRate":'+ overlay_frameRate +'}';
	//alert(encode_data);  
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/video/encode/channel/'+ id+'?r=' + Math.random(),
			dataType:'json',
			//contentType: "application/x-www-form-urlencoded; charset=UTF-8", 
			data:encode_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
				if(data.statusCode == 0){ 
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
					//alert('statusCode='+data.statusCode);
				}
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
				}
		})
}
function incoming_save_fbase()
{
	var incoming_data = '{"powerLineFrequencyMode": '+$('#encode_Fbase :selected').val()+'}';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/video/input/channel/1?r=' + Math.random(),
			dataType:'json',
			data:incoming_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
			audio_save();
			encode_save_content();
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
				}
		})
}

function audio_save()
{
	var audio_data = '{"enabled": '+$('#encodeType :selected').val()+'}';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/audio/encode/channel/101?r=' + Math.random(),
			dataType:'json',
			data:audio_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
			},
			error:function(a,b,c){ 

				}
		})
}

function ptz_common(ptz_step,ptz_act,ptz_speed)
{
	$.ajax({ 
		type:"PUT",
		url: dvr_url + '/cgi-bin/hi3510/ptzctrl.cgi?-step='+ ptz_step +'&-act='+ ptz_act +'&-speed='+ ptz_speed+'&r=' + Math.random(), 
		processData: false, 
		cache: false,
		data: "", 
		async:true,
		beforeSend: function(XMLHttpRequest){
			//alert("beforeSend");
		},
		success: function(data, textStatus){
			//alert(textStatus);
		},
		complete: function(XMLHttpRequest, textStatus){
			//alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert('error:'+textStatus);	
		}
	});
}
window.onload=function()
{
	var ptz_speed = 45;
	var ptz_step = 0;
	$('#ptz-up').mousedown(function(){
			var ptz_act = 'up';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			
			});
	$('#ptz-down').mousedown(function(){
			var ptz_act = 'down';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			});
	$('#ptz-left').mousedown(function(){
			var ptz_act = 'left';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			});
	$('#ptz-right').mousedown(function(){
			var ptz_act = 'right';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			});
	$('#ptz-cam').click(function(){
			var ptz_act = 'auto';
			ptz_common(ptz_step,ptz_act,ptz_speed);
		});
	$('#zoom-in').mousedown(function(){
			var ptz_act = 'focusin';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			});
	$('#zoom-out').mousedown(function(){
			var ptz_act = 'focusout';
			ptz_common(ptz_step,ptz_act,ptz_speed);
	}).mouseup(function(){
			var ptz_act = 'stop';
			ptz_common(ptz_step,ptz_act,ptz_speed);
			});
	$('#aaaa').mousedown(function(e){
				var ev = e || window.event;
				oWarp = $(this);
				X = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft;
				$('#aaaaa').width(X-119)
				ev.stopPropagation();
		$(document).mousemove(function(e){
				var ev = e || window.event;
				var x = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft;;
				x = x > oWarp.offset().left + oWarp.width() ? oWarp.offset().left + oWarp.width() : x;
				$('#aaaaa').width(x-119);
		}).mouseup(function(){
				$(document).off(); 
			})
			return false;
		})
}
function overlay_load_content()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	 $.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/video/encode/channel/"+nowChannl+'?r=' + Math.random(),
			dataType:"json",
			beforeSend : function(req){  
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){
					daX = data.datetimeOverlay.regionX,daY = data.datetimeOverlay.regionY;
					naX = data.channelNameOverlay.regionX,naY = data.channelNameOverlay.regionY;
					deX = data.deviceIDOverlay.regionX,deY = data.deviceIDOverlay.regionY;
					da_enabled = data.datetimeOverlay.enabled;
					na_enabled = data.channelNameOverlay.enabled;
					de_enabled = data.deviceIDOverlay.enabled;
					display_week = data.datetimeOverlay.displayWeek;
					O_channelName = data.channelName;
					time_format = data.datetimeOverlay.timeFormat;
					date_format = data.datetimeOverlay.dateFormat;
					overlay_data2ui();
					Overlay_region_load();
					$('input[id^="overlay1_"]').each(function(index){
						$(this)[0].onclick = function(){
							var obj = $('div.overlay_bg div').eq(index);
							obj.is(':visible') ? obj.hide() : obj.show();
						}
					})
					$('#overlay_DisplayWeek')[0].onclick = function(){
						var obj = $('#displayweek');
							obj.is(':visible') ? obj.hide() : obj.show();
					}
			}
		});
}
function overlay_data2ui()
{
	var snap = Math.random(); 
	$('#Overlay_background').find('div').remove();	
		var overlayBack = $('<div class="overlay_bg" style=" width: 100%; border:1px solid red; position: relative; z-index: 1; background-image:url(http://'+tmp_ip+'/snapshot?r='+ snap +');"></div>').appendTo($('#Overlay_background'));	
		/*$('div.overlay_bg').height( $('div.overlay_bg').width()*hP);*/	//自适应高度
		$('div.overlay_bg').height(360);
		$('div.overlay_bg').find('div').remove();
			var oFix1Left = parseInt((daX*610/100));
			var oFix1Top = parseInt((daY*328.5/100));
			var oFix2Left = parseInt((naX*610/100));
			var oFix2Top = parseInt((naY*328.5/100));
			var oFix3Left = parseInt((deX*610/100));
			var oFix3Top = parseInt((deY*328.5/100));
			dateX = daX;
			dateY = daY;
			nameX = naX;
			nameY = naY;
			deviceX = deX;
			deviceY = deY;
		var nowtime = $('#time_pc').val();
		var day=["Sun","Mon","Tue","Wed","Thu","Fri","Sat",]
		var Week = day[new Date().getDay()];

		var oFix1 = $('<div id="datetime" style="display:none; height:30px; width:210px;; border:1px solid gray; position:absolute; left:'+oFix1Left+'px; top:'+ oFix1Top +'px"><span style=" width:140px; height:30px; float:left;">'+nowtime+'</span><span id="displayweek" style=" float:right; display:none; width:60px;; height:30px; border:0px solid red;">'+Week+'</span></div>').appendTo($('div.overlay_bg'));
		var oFix2 = $('<div id="channelName" style="display:none; height:30px; width:100px; border:1px solid gray; position:absolute; left:'+oFix2Left+'px; top:'+ oFix2Top +'px">'+ O_channelName +'</div>').appendTo($('div.overlay_bg'));
		var oFix3 = $('<div id="play_id" style="display:none; height:30px; width:100px; border:1px solid gray; position:absolute; left:'+oFix3Left+'px; top:'+ oFix3Top +'px">ID:100001512</div>').appendTo($('div.overlay_bg'));
		
		$('#Overlay_dateFormat').val(date_format);
		$('#overlay_timeFormat').val(time_format);

		if(display_week == true){
			$('#overlay_DisplayWeek').prop('checked',true);
			$('#displayweek')[0].style.display='block';
		}else{
			$('#overlay_DisplayWeek').prop('checked',false);
			$('#displayweek')[0].style.display='none';
		}
		if(da_enabled == true){
			$('#overlay1_datetime').prop('checked',true);
			oFix1[0].style.display='block';
		}else{
			$('#overlay1_datetime').prop('checked',false);
			oFix1[0].style.display='none';
		}
		if(na_enabled == true){
			$('#overlay1_name').prop('checked',true);
			oFix2[0].style.display='block';
		}else{
			$('#overlay1_name').prop('checked',false);
		}
		if(de_enabled == true){
			$('#overlay1_id').prop('checked',true);
			oFix3[0].style.display='block';
		}else{
			$('#overlay1_id').prop('checked',false);
		}		
		if($('#overlay_enabled_1').val() == 'true'){
			$('#overlay_enabled_1').prop('checked',true);
		}else{
			$('#overlay_enabled_0').prop('checked',false);
		}		
		var oFix;
		fix(oFix1),fix(oFix2),fix(oFix3);
		function fix(oFix){
			oFix.mousedown(function(e){
				if(oFix[0].setCapture){
				oFix[0].setCapture();}
				else if(window.captureEvents)
				window.captureEvents(Event.MOUSEMOVE|Event.MOUSEUP);
				var ev = e || window.event;
				X = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft;
				Y = ev.clientY+document.body.scrollTop+document.documentElement.scrollTop;
				T = parseInt(oFix.css('top'));
				L = parseInt(oFix.css('left'));
				Overlay_region();
				$(document).mousemove(function(e){
						var ev = e || window.event;
							t = ev.clientY+document.body.scrollTop+document.documentElement.scrollTop - Y + T;
							t = t > $('div.overlay_bg').height() - oFix.height() - 2 ? $('div.overlay_bg').height() - oFix.height() -2 : t;
							t = t < 0 ? 0 : t;
							l = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft - X + L
							l = l > $('div.overlay_bg').width() - oFix.width() - 2 ?	$('div.overlay_bg').width() - oFix.width() -2  : l;
							l= l < 0 ? 0 : l;
						oFix.css({ 
							top: t,
							left: l
						})
						if(oFix == oFix1){
							dateX = parseInt(l*1000/610)/10 + 0.1;
							dateY = parseInt(t*1000/328.5)/10 + 0.1;
						}else if(oFix == oFix2){
							nameX = parseInt(l*1000/610)/10 + 0.1;
							nameY = parseInt(t*1000/328.5)/10 + 0.1;
						}else if(oFix == oFix3){
							deviceX = parseInt(l*1000/610)/10 + 0.1;
							deviceY = parseInt(t*1000/328.5)/10 + 0.1;	
						}
						Overlay_region();
						}).mouseup(function(){
								if(oFix[0].releaseCapture){
								oFix[0].releaseCapture();}
								else if(window.captureEvents)
								window.captureEvents(Event.MOUSEMOVE|Event.MOUSEUP);
								$(document).off();
								if(oFix == oFix1){
									dateX = parseInt(l*1000/610)/10 + 0.1;
									dateY = parseInt(t*1000/328.5)/10 + 0.1;
								}else if(oFix == oFix2){
									nameX = parseInt(l*1000/610)/10 + 0.1;
									nameY = parseInt(t*1000/328.5)/10 + 0.1;
								}else if(oFix == oFix3){
									deviceX = parseInt(l*1000/610)/10 + 0.1;
									deviceY = parseInt(t*1000/328.5)/10 + 0.1;	
								}
								Overlay_region();
						})
				})	
		}
}
function overlay_save_content()
{
	showInfo(langstr.save_setup);
	var overlay_DisplayWeek,overlay1_datetime,overlay1_name,overlay1_id;
	var Overlay_timeFormat = $('#overlay_timeFormat :selected').html();
	var Overlay_dateFormat =$('#Overlay_dateFormat :selected').html();
	if($('input#overlay_DisplayWeek:checked').val() == 'on')
	{
		overlay_DisplayWeek = true;
	}else
	{
		overlay_DisplayWeek = false;
	}
	if($('input#overlay1_datetime:checked').val() == 'on')
	{
		overlay1_datetime = true;
	}else
	{
		overlay1_datetime = false;
	}
	if($('input#overlay1_name:checked').val() == 'on')
	{
		overlay1_name = true;
	}else
	{
		overlay1_name = false;
	}
	if($('input#overlay1_id:checked').val() == 'on')
	{
		overlay1_id = true;
	}else
	{
		overlay1_id = false;
	}
	var Json = new Object;
	Json.datetimeOverlay = new Object;
	Json.datetimeOverlay.enabled= overlay1_datetime;
	Json.datetimeOverlay.dateFormat= Overlay_dateFormat;
	Json.datetimeOverlay.timeFormat= Overlay_timeFormat;
	Json.datetimeOverlay.regionX= dateX;
	Json.datetimeOverlay.regionY= dateY;
	Json.datetimeOverlay.displayWeek= overlay_DisplayWeek;
	Json.channelNameOverlay = new Object;
	Json.channelNameOverlay.enabled= overlay_DisplayWeek;
	Json.channelNameOverlay.regionX= nameX;
	Json.channelNameOverlay.regionY= nameY;
	Json.deviceIDOverlay = new Object;
	Json.deviceIDOverlay.enabled= overlay1_id;
	Json.deviceIDOverlay.irCutMode = deviceX;
	Json.deviceIDOverlay.irCutMode = deviceY;
	var overlay_data = $.toJSON(Json);
	//var overlay_data ='{"datetimeOverlay":{"enabled":'+overlay1_datetime+',"dateFormat":"'+Overlay_dateFormat+'","timeFormat":'+Overlay_timeFormat+',"regionX":'+dateX+',"regionY":'+dateY+',"displayWeek":'+overlay_DisplayWeek+'},"channelNameOverlay": {"enabled":'+overlay1_name+',"regionX": '+nameX+',"regionY": '+nameY+'},"deviceIDOverlay":{"enabled":'+overlay1_id+',"regionX":'+deviceX+',"regionY":'+deviceY+'}}';
	//alert(overlay_data);
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:'PUT',
			url:dvr_url + '/netsdk/video/encode/channel/101?r=' + Math.random(),
			dataType:'json',
			data:overlay_data,
			async:false,
			beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
			success:function(data){ 
				if(data.statusCode == 0){
					//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
					showInfo(langstr.save_success);
					setTimeout("hideInfo()",hide_delaytime_ms);
					//alert('statusCode='+data.statusCode);
				}
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
					}
		})
}
function Overlay_region_load(){
	$('#overlay_datetime_region').html('(&nbsp;'+parseInt(daX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(daY*10)/10 +'&nbsp;)');
	$('#overlay_channelName_region').html('(&nbsp;'+parseInt(naX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(naY*10)/10+'&nbsp;)');
	$('#overlay_deviceID_region').html('(&nbsp;'+parseInt(deX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(deY*10)/10+'&nbsp;)');
	}
function Overlay_region(){
	$('#overlay_datetime_region').html('(&nbsp;'+parseInt(dateX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(dateY*10)/10+'&nbsp;)');
	$('#overlay_channelName_region').html('(&nbsp;'+parseInt(nameX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(nameY*10)/10+'&nbsp;)');
	$('#overlay_deviceID_region').html('(&nbsp;'+parseInt(deviceX*10)/10 +'&nbsp;,&nbsp;&nbsp;'+ parseInt(deviceY*10)/10+'&nbsp;)');	
	}
//Cover
function cover_load_content()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/video/input/channel/1/privacyMasks?r=" + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){
				//id=1
					P_id = data[0].id;
					P_enabled = data[0].enabled;
					P_X = data[0].regionX;
					P_Y = data[0].regionY;
					P_W = data[0].regionWidth;
					P_H = data[0].regionHeight;
					P_color = data[0].regionColor;
					$('#Cover_bgColor').val(data[0].regionColor);
				//id=2
					P1_id = data[1].id;
					P1_enabled = data[1].enabled;
					P1_X = data[1].regionX;
					P1_Y = data[1].regionY;
					P1_W = data[1].regionWidth;
					P1_H = data[1].regionHeight;
					P1_color = data[1].regionColor;
				//id=3
					P2_id = data[2].id;
					P2_enabled = data[2].enabled;
					P2_X = data[2].regionX;
					P2_Y = data[2].regionY;
					P2_W = data[2].regionWidth;
					P2_H = data[2].regionHeight;
					P2_color = data[2].regionColor;
				//id=4
					P3_id = data[3].id;
					P3_enabled = data[3].enabled;
					P3_X = data[3].regionX;
					P3_Y = data[3].regionY;
					P3_W = data[3].regionWidth;
					P3_H = data[3].regionHeight;
					P3_color = data[3].regionColor;
				
					cover_data2ui();
				}
			});
}
function cover_data2ui()
{	
		var snap = Math.random();
		var X,Y,disX,disY;
		$('#Cover_background').find('div').remove();	
			var coverBack = $('<div class="cover_bg" style=" width: 100%; border:1px solid red; position: relative; z-index: 1; background:url(http://'+tmp_ip+'/snapshot?r='+ snap +') no-repeat center center;"></div>').appendTo($('#Cover_background'));	
			$('div.cover_bg').height(360);	
			$('div.cover_bg').find('div').remove();
			W = parseInt(P_W*6.4),H = parseInt(P_H*3.6),t = parseInt(P_Y*3.6),l = parseInt(P_X*6.4);
			W1 = parseInt(P1_W*6.4),H1 = parseInt(P1_H*3.6),t1 = parseInt(P1_Y*3.6),l1 = parseInt(P1_X*6.4);
			W2 = parseInt(P2_W*6.4),H2 = parseInt(P2_H*3.6),t2 = parseInt(P2_Y*3.6),l2 = parseInt(P2_X*6.4);
			W3 = parseInt(P3_W*6.4),H3 = parseInt(P3_H*3.6),t3 = parseInt(P3_Y*3.6),l3 = parseInt(P3_X*6.4);
			var Bgcolor = '#' + P_color;
		oMove_view = $('<div id="oView" style=" display:none; height:'+H+'px; width:'+W+'px; background-color:'+ Bgcolor +'; border:1px solid red; position:absolute; left:'+l+'px; top:'+t+'px"></div>').appendTo($('div.cover_bg'));
		oMove_view1 = $('<div id="oView1" style=" display:none; height:'+H1+'px; width:'+W1+'px; background-color:'+ Bgcolor +'; border:1px solid red; position:absolute; left:'+l1+'px; top:'+t1+'px"></div>').appendTo($('div.cover_bg'));
		oMove_view2 = $('<div id="oView2" style=" display:none; height:'+H2+'px; width:'+W2+'px; background-color:'+ Bgcolor +'; border:1px solid red; position:absolute; left:'+l2+'px; top:'+t2+'px"></div>').appendTo($('div.cover_bg'));
		oMove_view3 = $('<div id="oView3" style=" display:none; height:'+H3+'px; width:'+W3+'px; background-color:'+ Bgcolor +'; border:1px solid red; position:absolute; left:'+l3+'px; top:'+t3+'px"></div>').appendTo($('div.cover_bg'));

		if(P3_enabled == true){
			$('#cover_enabled').prop('checked',true);
			
		}else if(P2_enabled == true){
			$('#cover_enabled').prop('checked',true);		
		}
		else if(P1_enabled == true){
			$('#cover_enabled').prop('checked',true);
			
		}else if(P_enabled == true){
			$('#cover_enabled').prop('checked',true);
		}
		else{
			$('#cover_enabled').prop('checked',false);
		}
	 	var i =0;
		$('div.cover_bg').mousedown(function(e){
				var ev = e || window.event;
				if(i>3){
				oWarp.find('div').remove();
				i=0;
				}
				i+=1;
				oWarp = $(this);
				//oWarp.find('div').remove();
				X = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft;
				Y = ev.clientY+document.body.scrollTop+document.documentElement.scrollTop;
				oMove = $('<div id="Move'+i+'" style="height:1px; width:1px; border:1px solid red; background-color:'+ Bgcolor +'; position:absolute; left:'+(X - oWarp.offset().left)+'px; top:'+(Y - oWarp.offset().top)+'px"></div>').appendTo(oWarp);
				ev.stopPropagation();
		$(document).mousemove(function(e){
				var ev = e || window.event;
				var x = ev.clientX+document.body.scrollLeft+document.documentElement.scrollLeft;;
				x = x > oWarp.offset().left + oWarp.width() ? oWarp.offset().left + oWarp.width() : x;
				var y = ev.clientY+document.body.scrollTop+document.documentElement.scrollTop;
				y = y > oWarp.offset().top + oWarp.height() ? oWarp.offset().top + oWarp.height() : y;
				var W = Math.abs(x - X);
				var H = Math.abs(y - Y);
				var L = x - X < 0 ? x - oWarp.offset().left : parseInt(oMove.css('left'));
					if(L < 0){ 
						L = 0;
						W = Math.abs(X - oWarp.offset().left)
					}
				var T = y - Y < 0 ? y - oWarp.offset().top : parseInt(oMove.css('top'));
					if(T < 0){ 
						T = 0;
						H = Math.abs(Y - oWarp.offset().top)
					}
				$('.test1').html(W+':'+H);
				oMove.css({ 
					width:W,
					height:H,
					top:T,
					left:L
				})
		}).mouseup(function(){
				$(document).off(); 
			})
			return false;
		})
}
//强制保留1为小数
function toDecimal(x) {  
            var f = parseFloat(x);  
            if (isNaN(f)) {  
                return false;  
            }  
            var f = Math.round(x*10)/10;  
            var s = f.toString();  
            var rs = s.indexOf('.');  
            if (rs < 0) {  
                rs = s.length;  
                s += '.';  
            }  
            while (s.length <= rs + 1) {  
                s += '0';  
            }  
            return s;  
} 
function cover_save_content()
{
	showInfo(langstr.save_setup);
	var cover_enabled,Cover_bgColor;
	switch($("#Cover_bgColor")[0].selectedIndex)
	{
		case 0: Cover_bgColor = "ffffff";break;
		case 1: Cover_bgColor = "5AAF5A";break;
		case 2: Cover_bgColor = "006ECF";break;
		default:break;
	}
	if($('input#cover_enabled:checked').val() == 'on')
	{
		cover_enabled = true;
	}else
	{
		cover_enabled = false;
	}   
	var cover_data,cover_data1,cover_data2,cover_data3,cover_data4;
	if($('#Move1').width() != null && $('#Move2').width() == null)
	{
		cover_data1 = '"id":1,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move1').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move1').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move1').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move1').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data ='[{'+cover_data1+'},{"id":2,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0},{"id":3,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0},{"id":4,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0}]';	
	}
	else if($('#Move2').width() != null && $('#Move3').width() == null)
	{
		cover_data1 = '"id":1,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move1').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move1').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move1').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move1').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data2 = '"id":2,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move2').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move2').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move2').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move2').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data ='[{'+cover_data1+'},{'+cover_data2+'},{"id":3,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0},{"id":4,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0}]';	
	}
	else if($('#Move3').width() != null && $('#Move4').width() == null)
	{
		cover_data1 = '"id":1,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move1').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move1').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move1').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move1').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data2 = '"id":2,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move2').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move2').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move2').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move2').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data3 = '"id":3,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move3').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move3').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move3').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move3').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data ='[{'+cover_data1+'},{'+cover_data2+'},{'+cover_data3+'},{"id":4,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0},{"id":4,"enabled":false,"regionX":0.0,"regionY":0.0,"regionWidth":0.0,"regionHeight":0.0}]';	
	}
	else if($('#Move4').width() != null)
	{
		cover_data1 = '"id":1,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move1').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move1').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move1').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move1').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data2 = '"id":2,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move2').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move2').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move2').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move2').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data3 = '"id":3,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move3').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move3').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move3').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move3').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data4 = '"id":4,"enabled":'+cover_enabled+',"regionX":'+toDecimal(parseInt($('#Move4').offset().left-oWarp.offset().left)*100/640)+',"regionY":'+toDecimal(parseInt($('#Move4').offset().top-oWarp.offset().top)*100/360)+',"regionWidth":'+toDecimal(parseInt($('#Move4').width())*100/640)+',"regionHeight":'+toDecimal(parseInt($('#Move4').height())*100/360)+',"regionColor":"'+Cover_bgColor+'"';
		cover_data ='[{'+cover_data1+'},{'+cover_data2+'},{'+cover_data3+'},{'+cover_data4+'}]';
	}
	else
	{
		cover_data ='[{"id":1,"enabled":'+cover_enabled+',"regionColor":"'+Cover_bgColor+'"},{"id":2,"enabled":'+cover_enabled+',"regionColor":"'+Cover_bgColor+'"},{"id":3,"enabled":'+cover_enabled+',"regionColor":"'+Cover_bgColor+'"},{"id":4,"enabled":'+cover_enabled+',"regionColor":"'+Cover_bgColor+'"}]';
	}
	//alert(cover_data);
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
		type:'PUT',
		url:dvr_url + '/netsdk/video/input/channel/1/privacyMasks?r=' + Math.random(),
		dataType:'json',
		data:cover_data,
		async:false,
		beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
		success:function(data){ 
			if(data.statusCode == 0){
				//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
				showInfo(langstr.save_success);
				setTimeout("hideInfo()",hide_delaytime_ms); 
				cover_load_content();
			}
		},
		error:function(a,b,c){ 
			if(a.status == 401){
				alert(langstr.setting_permit);
			}else{
				alert(langstr.setting_wait);	
			}
			setTimeout("hideInfo()",500);
		}
	})
}
//remote
function remote_load_content()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/Network/Interface/1?r=" + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){
				data.ddns.enabled == true ?$('#remote_network_ddns_1').prop('checked',true):$('#remote_network_ddns_0').prop('checked',true);
				$('#remote_network_ddns_provider').val(data.ddns.ddnsProvider);
				$('#remote_network_ddns_url').val(data.ddns.ddnsUrl);
				$('#remote_network_ddns_usr').val(data.ddns.ddnsUserName);
				$('#remote_network_ddns_pwd').val(data.ddns.ddnsPassword);
				data.pppoe.enabled == true ?$('#remote_network_pppoe_1').prop('checked',true):$('#remote_network_pppoe_0').prop('checked',true);
				$('#remote_network_pppoe_usr').val(data.pppoe.pppoeUserName);
				$('#remote_network_pppoe_pwd').val(data.pppoe.pppoePassword);
			}
		});
}	
function remote_save_content()
{
	var pppoe_enabled = $('#remote_network_pppoe_1').prop('checked') ?true:false;
	var ddns_enabled = $('#remote_network_ddns_1').prop('checked') ?true:false;
	var Json = new Object;
	Json.pppoe = new Object;
	Json.pppoe.enabled = pppoe_enabled;
	Json.pppoe.pppoeUserName = $('#remote_network_pppoe_usr').val();
	Json.pppoe.pppoePassword = $('#remote_network_pppoe_pwd').val();
	Json.ddns = new Object;
	Json.ddns.enabled = ddns_enabled;
	Json.ddns.ddnsProvider = $('#remote_network_ddns_provider :selected').html();
	Json.ddns.ddnsUrl = $('#remote_network_ddns_url').val();
	Json.ddns.ddnsUserName = $('#remote_network_ddns_usr').val();
	Json.ddns.ddnsPassword = $('#remote_network_ddns_pwd').val();
	
	var data = $.toJSON(Json);
	//var data = '{"pppoe": { "enabled": '+pppoe_enabled+', "pppoeUserName": "'+$('#remote_network_pppoe_usr').val()+'", "pppoePassword": "'+$('#remote_network_pppoe_pwd').val()+'" }, "ddns": { "enabled": '+ddns_enabled+', "ddnsProvider": "'+$('#remote_network_ddns_provider :selected').html()+'", "ddnsUrl": "'+$('#remote_network_ddns_url').val()+'", "ddnsUserName": "'+$('#remote_network_ddns_usr').val()+'", "ddnsPassword": "'+$('#remote_network_ddns_pwd').val()+'" }}';
	//alert(data)
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:'PUT',
			url:dvr_url + "/netsdk/Network/Interface/1?r=" + Math.random(),
			dataType:'json',
			data: data,
			async:false,
			beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){ 
				//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
				showInfo(langstr.save_success);
				setTimeout("hideInfo()",hide_delaytime_ms); 
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		})
}
function remote_change()
{
	if($("#remote_network_ddns_1")[0].checked == 1)
	{
		$("#remote_network_ddns_provider")[0].disabled = false;			
		$("#remote_network_ddns_url")[0].disabled = false;			
		$("#remote_network_ddns_usr")[0].disabled = false;			
		$("#remote_network_ddns_pwd")[0].disabled = false;			
	}
	else
	{
		$("#remote_network_ddns_provider")[0].disabled = true;			
		$("#remote_network_ddns_url")[0].disabled = true;			
		$("#remote_network_ddns_usr")[0].disabled = true;			
		$("#remote_network_ddns_pwd")[0].disabled = true;				
	}

	if($("#remote_network_pppoe_1")[0].checked == 1)
	{
		$("#remote_network_pppoe_usr")[0].disabled = false;			
		$("#remote_network_pppoe_pwd")[0].disabled = false;			
	}
	else
	{
		$("#remote_network_pppoe_usr")[0].disabled = true;			
		$("#remote_network_pppoe_pwd")[0].disabled = true;			
	}
}
//local
function local_load_content()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	var jsonp;
	//var auth = "Basic " + base64.encode('admin:');
	
	/**
	 * Get MAC Address. 
	 */
	NetSDK_User(g_usr, g_pwd);
	NetSDK_GetJSON("/NetSDK/System/deviceInfo", function(Obj){
		$('#local_network_mac').val(Obj.macAddress);
		$('#local_network_p2p_id').val(Obj.eseeID);
	});
	
	/**
	 * Get P2P. 
	 */
	NetSDK_GetJSON("/NetSDK/Network/ESee", function(Obj){
		$('#local_network_p2p').prop("checked", Obj.enabled);
	});
	
	
	/**
	 * Get Ethernet. 
	 */
	NetSDK_GetJSON("/NetSDK/Network/Interface/1", function(Obj){
		switch(Obj.lan.addressingType) {
			case "static":
				//$('#local_network_dhcp').prop("checked", false);
				$("#local_network_ipautoadpatation_0")[0].checked = 1;
				$("#local_network_ipautoadpatation_1")[0].checked = 0;
				break;
			case "dynamic":	
				//$('#local_network_dhcp').prop("checked", true);
				$("#local_network_ipautoadpatation_1")[0].checked = 1;
				$("#local_network_ipautoadpatation_0")[0].checked = 0;
				break;
		}
		$('#local_network_ip').val(Obj.lan.staticIP);
		$('#local_network_gateway').val(Obj.lan.staticGateway);
		$('#local_network_submask').val(Obj.lan.staticNetmask);
	});
	
	/**
	 * Get DNS. 
	 */
	NetSDK_GetJSON("/NetSDK/Network/DNS", function(Obj){
		$('#local_network_dns').val(Obj.preferredDns);
		$('#local_network_dns2').val(Obj.staticAlternateDns);
	});

}

function local_load_4()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/Network/Port/1?r=" + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){
				$('#local_network_port').val(data.value);
			}
		});
}
function local_save_content()
{	
	var reqObj = new Object;
	reqObj.macAddress = $('#local_network_mac').val();
//	var macAddress_data = '{"macAddress": "'+$('#local_network_mac').val()+'"}';
	var macAddress_data = $.toJSON(reqObj);
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:'PUT',
			url:dvr_url + "/netsdk/System/deviceInfo?r=" + Math.random(),
			dataType:'json',
			data: macAddress_data,
			async:false,
			beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){ 
				local_save_1();
				local_save_2();
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		})
}
function local_save_1()
{				
	var addressingType = $('#local_network_ipautoadpatation_1').prop('checked') ?'dynamic':'static';
	if($("#local_network_ipautoadpatation_1")[0].checked ==1)
	{
		$("#local_network_ipautoadpatation_0")[0].checked = 0;
	}
	if($("#local_network_ipautoadpatation_0")[0].checked ==1)
	{
		$("#local_network_ipautoadpatation_1")[0].checked = 0;
	}
	//var upnp_enabled = $('#local_network_upnp_1').prop('checked') ?true:false;
//	var addressingType =$('#local_network_dhcp01').val();
	//var addressingType =$('input[name="dhcp"]:checked').val();
//	var upnp_enabled =$('#local_network_upnp01').val();
//	var local_save_data = '{ "lan": {"addressingType": "'+addressingType+'", "staticIP": "'+$('#local_network_ip').val()+'", "staticNetmask": "'+$('#local_network_submask').val()+'", "staticGateway": "'+$('#local_network_gateway').val()+'" }, "upnp": { "enabled": '+upnp_enabled+' }}';
	var Json = new Object;
	Json.lan = new Object;
	Json.lan.addressingType = addressingType;
	Json.lan.staticIP = $('#local_network_ip').val();
	Json.lan.staticNetmask = $('#local_network_submask').val();
	Json.lan.staticGateway = $('#local_network_gateway').val();
	var local_save_data = $.toJSON(Json);
	//var local_save_data = '{ "lan": {"addressingType": "'+addressingType+'", "staticIP": "'+$('#local_network_ip').val()+'", "staticNetmask": "'+$('#local_network_submask').val()+'", "staticGateway": "'+$('#local_network_gateway').val()+'" }}';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:'PUT',
			url:dvr_url + "/netsdk/Network/Interface/1?r=" + Math.random(),
			dataType:'json',
			data: local_save_data,
			async:false,
			beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){ 
				local_save_3();
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		})
}

function local_save_2()
{	
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	
	var Json = new Object;
	Json.enabled = $('#local_network_p2p').prop('checked');
	var jsonp = $.toJSON(Json);
	
	$.ajax({
		type:'PUT',
		url:dvr_url + "/netsdk/Network/Esee?r=" + Math.random(),
		dataType:'json',
		data: jsonp,
		async: false,
		beforeSend : function(req ) {
			req .setRequestHeader('Authorization', auth);
			},
		success:function(data){ 
			//local_save_3();
		},
		error:function(a,b,c){ 
			if(a.status == 401){
				alert(langstr.setting_permit);
			}else{
				alert(langstr.setting_wait);	
			}
			setTimeout("hideInfo()",500);
		}
	})
}
function local_save_3()
{
	var Json = new Object;
	Json.preferredDns =$('#local_network_dns').val();
	Json.staticAlternateDns = $('#local_network_dns2').val();
	var data = $.toJSON(Json);
	//var data = '{ "preferredDns": "'+$('#local_network_dns').val()+'", "staticAlternateDns": "'+$('#local_network_dns2').val()+'" }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:'PUT',
			url:dvr_url + "/netsdk/Network/Dns?r=" + Math.random(),
			dataType:'json',
			data: data,
			async:false,
			beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){ 
				//local_save_4();
				
				showInfo(langstr.save_success);
				setTimeout("hideInfo()",hide_delaytime_ms); 
			    settime();
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		})
}
function local_save_4()
{
	var data = '{"value": '+$('#local_network_port').val()+' }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:'PUT',
			url:dvr_url + "/netsdk/Network/Port/1?r=" + Math.random(),
			dataType:'json',
			data: data,
			async:false,
			beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){ 
				//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
				showInfo(langstr.save_success);
				setTimeout("hideInfo()",hide_delaytime_ms); 
			    settime();
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});
}
var countdown=10; 
function settime() { 
if (countdown == 0) { 
	//self.location.replace("http://"+$('#local_network_ip').val()+":"+$('#local_network_port').val());
	/// FIXME 暂时强制 80。
	self.location.replace("http://"+$('#local_network_ip').val());
} else { 
	$('#losaco').attr("disabled", true); 
	$('#lai1').css("display","block");
	$('#lai1').text(langstr.save_set+"("+countdown+")"); 
    countdown--; 
} 
setTimeout(function() { 
settime() 
},1000) 
} 
var default_mac;
var default_ip;
var default_port;
function save_mac_value()
{
	var mac_obj=document.getElementById('local_network_mac');
	default_mac=mac_obj.value;
}
function is_valid_mac()
{
	//mac地址正则表达嶍
	var mac_obj=document.getElementById('local_network_mac');
	var reg_name=/[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}-[A-F\d]{2}/; 
	var reg_name_s=/[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}-[a-f\d]{2}/; 
	if ((!reg_name.test(mac_obj.value)) && (!reg_name_s.test(mac_obj.value)))
	{ 
		alert(langstr.format_wrong+"22-24-21-19-BD-E4"); 
		mac_obj.value=default_mac;
		return false; 
	} 
	return true; 
}
function save_ipaddr(id)
{
	var ipaddr_obj=document.getElementById(id);
	default_ip=ipaddr_obj.value;
}
function save_port()
{
	var port_obj=document.getElementById('local_network_port');
	default_port=port_obj.value;
}
function is_valid_ip(id)
{
	var ip_obj=document.getElementById(id);
	var re=/^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;//正则表达嶿  
	if(re.test(ip_obj.value))     
	{     
	   if( RegExp.$1<256 && RegExp.$2<256 && RegExp.$3<256 && RegExp.$4<256)   
	   return true;     
	}     
	alert(langstr.format_wrong+"192.168.1.234");     
	ip_obj.value=default_ip;
	return false;   
}
function is_valid_port()
{
	var port_obj=document.getElementById('local_network_port');
	if(isNaN(port_obj.value)==true)
	{
		alert(langstr.port_wrong);
		port_obj.value=default_port;
	}
	return true;
}

function ip_config_change()
{
	/*if($("#local_network_dhcp_1")[0].checked ==1)
	{
		//alert("use dhcp");
		$("#local_network_ip")[0].disabled = true;
		$("#local_network_gateway")[0].disabled = true;
		$("#local_network_submask")[0].disabled = true;
		$("#local_network_dns")[0].disabled = true;
		$("#local_network_dns2")[0].disabled = true;
		$("#local_network_port")[0].disabled = true;
	}
	else
	{
		//alert("use static ip");
		$("#local_network_ip")[0].disabled = false;
		$("#local_network_gateway")[0].disabled = false;
		$("#local_network_submask")[0].disabled = false;
		$("#local_network_dns")[0].disabled = false;
		$("#local_network_dns2")[0].disabled = false;
		$("#local_network_port")[0].disabled = false;
	}*/
}
//wireless
function ap_dhcp_change()
{
	if($("#dhcpServer_enabled_1")[0].checked ==1)
	{
		//alert("use dhcp");
		$("#dhcpIpRange")[0].disabled = true;
		$("#dhcpIpNumber")[0].disabled = true;
		$("#dhcpIpDns")[0].disabled = true;
		$("#dhcpIpGateway")[0].disabled = true;
	}
	else
	{
		//alert("use static ip");
		$("#dhcpIpRange")[0].disabled = false;
		$("#dhcpIpNumber")[0].disabled = false;
		$("#dhcpIpDns")[0].disabled = false;
		$("#dhcpIpGateway")[0].disabled = false;
	}
}
function sta_dhcp_change()
{
	if($("#dhcpAutoSettingEnabled_1")[0].checked ==1)
	{
		//alert("use dhcp");
		$("#wlan0_staticIP_sta")[0].disabled = true;
		$("#wlan0_staticNetmask_sta")[0].disabled = true;
	}
	else
	{
		//alert("use static ip");
		$("#wlan0_staticIP_sta")[0].disabled = false;
		$("#wlan0_staticNetmask_sta")[0].disabled = false;
	}
}

var set_wireless_mode = true;
function wireless_load_content()
{
	var aa = $('#wireless_mode').val();
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/Network/Interface/4/Wireless?r=" + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){
				if(set_wireless_mode == true){
					$('#wireless_mode').val(data.wirelessMode);//stationMode
					aa = $('#wireless_mode').val();
					set_wireless_mode = false;
				}
				if(aa == 'accessPoint')
				{
					$('#sta')[0].style.display='none';
					$('#acc')[0].style.display='block';
				}
				if(aa == 'stationMode')
				{
					$('#acc')[0].style.display='none';
					$('#sta')[0].style.display='block';
				}
				if(aa == 'none')
				{
					$('#sta')[0].style.display='none';
					$('#acc')[0].style.display='none';
				}
					//stationMode
					$('#wirelessApBssId_sta').val(data.stationMode.wirelessApBssId);
					$('#wirelessApEssId_sta').val(data.stationMode.wirelessApEssId);
					$('#wirelessApPsk_sta').val(data.stationMode.wirelessApPsk);
					$('#wirelessStaMode_sta').val(data.stationMode.wirelessStaMode);
					//accessPointMode
					$('#wirelessApBssId_acc').val(data.accessPointMode.wirelessBssId);
					$('#wirelessApEssId_acc').val(data.accessPointMode.wirelessEssId);
					$('#wirelessApPsk_acc').val(data.accessPointMode.wirelessPsk);
					$('#wirelessApMode_acc').val(data.accessPointMode.wirelessApMode);
					$('#wirelessApMode80211nChannel_acc').val(data.accessPointMode.wirelessApMode80211nChannel);
					data.accessPointMode.wirelessEssIdBroadcastingEnabled == true ?$('#wirelessEssIdBroadcastingEnabled_1').prop('checked',true):$('#wirelessEssIdBroadcastingEnabled_0').prop('checked',true);
					$('#wirelessWpaMode_acc').val(data.accessPointMode.wirelessWpaMode);
					data.dhcpServer.enabled == true ?$('#dhcpServer_enabled_1').prop('checked',true):$('#dhcpServer_enabled_0').prop('checked',true);
					data.dhcpServer.dhcpAutoSettingEnabled == true ?$('#dhcpAutoSettingEnabled_1').prop('checked',true):$('#dhcpAutoSettingEnabled_0').prop('checked',true);
					$('#dhcpIpRange').val(data.dhcpServer.dhcpIpRange);
					$('#dhcpIpNumber').val(data.dhcpServer.dhcpIpNumber);
					$('#dhcpIpDns').val(data.dhcpServer.dhcpIpDns);
					$('#dhcpIpGateway').val(data.dhcpServer.dhcpIpGateway);
					data.stationMode.wirelessFixedBpsModeEnabled == true ?$('#wireless_sta_1').prop('checked',true):$('#wireless_sta_0').prop('checked',true);
					sta_dhcp_change();
					//ap_dhcp_change();
					wireless_load_content_1();
				
			},
				error:function(a,b,c){ 
					if(a.status != 200){
					$('#sta')[0].style.display='none';
					$('#acc')[0].style.display='none';
					$('#wireless_mode').val("none");
					}
			}
		});
}
function wireless_load_content_1()
{

	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
			type:"GET",
			url:dvr_url + "/netsdk/Network/Interface/4?r=" + Math.random(),
			dataType:"json",
			beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
			success:function(data){
				$('#wlan0_staticIP_ap').val(data.lan.staticIP);
				$('#wlan0_staticNetmask_ap').val(data.lan.staticNetmask);
				$('#wlan0_staticIP_sta').val(data.lan.staticIP);
				$('#wlan0_staticNetmask_sta').val(data.lan.staticNetmask);
				$('#wlan0_staticGateway_sta').val(data.lan.staticGateway);

				if(data.lan.addressingType == "dynamic")
				{
					$('#dhcpAutoSettingEnabled_1').prop('checked',true);
					$('#dhcpAutoSettingEnabled_0').prop('checked',false);
				}else{
					$('#dhcpAutoSettingEnabled_1').prop('checked',false);
					$('#dhcpAutoSettingEnabled_0').prop('checked',true);
				}
	 	}
		});
}
function wireless_save_content()
{
	var wireless_mode = $('#wireless_mode').val();
	var wirelessStaMode_sta = $('#wirelessStaMode_sta').val();
	var wirelessApBssId_sta = $('#wirelessApBssId_sta').val();
	var wirelessApEssId_sta = $('#wirelessApEssId_sta').val();
	var wirelessApPsk_sta = $('#wirelessApPsk_sta').val();
	var wirelessStaFixedBpsModeEnable = $('#wireless_sta_1').prop('checked') ?true:false;
 
	var wirelessApBssId_acc = $('#wirelessApBssId_acc').val();
	var wirelessApEssId_acc = $('#wirelessApEssId_acc').val();
	var wirelessApPsk_acc = $('#wirelessApPsk_acc').val();
	var wirelessApMode_acc = $('#wirelessApMode_acc').val();
	var wirelessApMode80211nChannel_acc = $('#wirelessApMode80211nChannel_acc').val();
	var	wirelessEssIdBroadcastingEnabled = $('#wirelessEssIdBroadcastingEnabled_1').prop('checked') ?true:false;
	var wirelessWpaMode_acc = $('#wirelessWpaMode_acc').val();
	var	dhcpServer_enabled = $('#dhcpServer_enabled_1').prop('checked') ?true:false;
	var	dhcpAutoSettingEnabled = $('#dhcpAutoSettingEnabled_1').prop('checked') ?true:false;
	var dhcpIpRange = $('#dhcpIpRange').val();
	var dhcpIpNumber = $('#dhcpIpNumber').val();
	var dhcpIpDns = $('#dhcpIpDns').val();
	var dhcpIpGateway = $('#dhcpIpGateway').val();
	
	var Json = new Object;
	Json.wirelessMode = wireless_mode;
	
	Json.stationMode = new Object;
	Json.stationMode.wirelessStaMode = wirelessStaMode_sta;
	Json.stationMode.wirelessApBssId = wirelessApBssId_sta;
	Json.stationMode.wirelessApEssId = wirelessApEssId_sta;
	Json.stationMode.wirelessApPsk = wirelessApPsk_sta;
	Json.stationMode.wirelessFixedBpsModeEnabled = wirelessStaFixedBpsModeEnable;
	
	Json.accessPointMode = new Object;
	Json.accessPointMode.wirelessBssId = wirelessApBssId_acc;
	Json.accessPointMode.wirelessApMode = wirelessApMode_acc;
	Json.accessPointMode.wirelessEssId = wirelessApEssId_acc;
	Json.accessPointMode.wirelessPsk = wirelessApPsk_acc;
	Json.accessPointMode.wirelessEssIdBroadcastingEnabled = wirelessEssIdBroadcastingEnabled;
	Json.accessPointMode.wirelessWpaMode = wirelessWpaMode_acc;
	Json.accessPointMode.wirelessApMode80211nChannel = wirelessApMode80211nChannel_acc;
	
	Json.dhcpServer = new Object;
	Json.dhcpServer.enabled = dhcpServer_enabled;
	Json.dhcpServer.dhcpAutoSettingEnabled = dhcpAutoSettingEnabled;
	Json.dhcpServer.dhcpIpRange = dhcpIpRange;
	Json.dhcpServer.dhcpIpNumber = dhcpIpNumber;
	Json.dhcpServer.dhcpIpDns = dhcpIpDns;
	Json.dhcpServer.dhcpIpGateway = dhcpIpGateway;

	var wireless_data = $.toJSON(Json);
	//var wireless_data = '{ "wirelessMode": "'+wireless_mode+'", "stationMode": { "wirelessStaMode": "'+wirelessStaMode_sta+'", "wirelessApBssId": "'+wirelessApBssId_sta+'", "wirelessApEssId": "'+wirelessApEssId_sta+'", "wirelessApPsk": "'+wirelessApPsk_sta+'", "wirelessFixedBpsModeEnabled": '+wirelessStaFixedBpsModeEnable+' }, "accessPointMode": { "wirelessBssId": "'+wirelessApBssId_acc+'", "wirelessEssId": "'+wirelessApEssId_acc+'", "wirelessPsk": "'+wirelessApPsk_acc+'", "wirelessApMode": "'+wirelessApMode_acc+'", "wirelessApMode80211nChannel": "'+wirelessApMode80211nChannel_acc+'", "wirelessEssIdBroadcastingEnabled": '+wirelessEssIdBroadcastingEnabled+', "wirelessWpaMode": "'+wirelessWpaMode_acc+'" }, "dhcpServer": { "enabled": '+dhcpServer_enabled+', "dhcpAutoSettingEnabled": '+dhcpAutoSettingEnabled+', "dhcpIpRange": "'+dhcpIpRange+'", "dhcpIpNumber": "'+dhcpIpNumber+'", "dhcpIpDns": "'+dhcpIpDns+'", "dhcpIpGateway": "'+dhcpIpGateway+'" } }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
		type:'PUT',
		url:dvr_url + "/netsdk/Network/Interface/4/Wireless?r=" + Math.random(),
		dataType:'json',
		data:wireless_data,
		async:false,
		beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
		success:function(data){ 
			wireless_save_content_1();
		},
		error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
		}
	})
}
function wireless_save_content_1()
{
	var wireless_mode = $('#wireless_mode').val();
	var wlan0_staticIP = $('#wlan0_staticIP_ap').val();
	var wlan0_staticNetmask = $('#wlan0_staticNetmask_ap').val();
	var wlan0_staticGateway = $('#wlan0_staticGateway_ap').val();
	var wlan0_addressingType;
	if($('#dhcpAutoSettingEnabled_1').prop('checked')  ==  1)
	{
	 	 wlan0_addressingType ="dynamic";
	}else{
		wlan0_addressingType ="static";
	}
	if(wireless_mode == "accessPoint"){
		 wlan0_staticIP = $('#wlan0_staticIP_ap').val();
		wlan0_staticNetmask = $('#wlan0_staticNetmask_ap').val();
		wlan0_staticGateway = $('#wlan0_staticGateway_ap').val();
	}else if(wireless_mode == "stationMode"){
		wlan0_staticIP = $('#wlan0_staticIP_sta').val();
		wlan0_staticNetmask = $('#wlan0_staticNetmask_sta').val();
		wlan0_staticGateway = $('#wlan0_staticGateway_sta').val();
	}else{
		
	}
	
	var Json = new Object;
	Json.lan = new Object;
	Json.lan.staticIP = wlan0_staticIP;
	Json.lan.staticNetmask = wlan0_staticNetmask;
	Json.lan.staticGateway = wlan0_staticGateway;
	Json.lan.addressingType = wlan0_addressingType;
	var wlan0_data = $.toJSON(Json);
	//var wlan0_data = '{"lan": {"staticIP": "'+wlan0_staticIP+'", "staticNetmask": "'+wlan0_staticNetmask+'", "staticGateway": "'+wlan0_staticGateway+'" } }';
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
		type:'PUT',
		url:dvr_url + "/netsdk/Network/Interface/4?r=" + Math.random(),
		dataType:'json',
		data:wlan0_data,
		async:false,
		beforeSend : function(req ) {
        	req .setRequestHeader('Authorization', auth);
    		},
		success:function(data){ 
		},
		error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
		}
	})
}
//alarmin
function alarmin_data2ui(dvr_data)
{
}
function alarmin_load_content()
{
}	
function alarmin_save_content()
{
}

//capture
function capture_data2ui(dvr_data)
{
}
function capture_load_content()
{
}		
function capture_save_content()
{
}

//ptz
function ptz_data2ui(dvr_data)
{
}
function ptz_load_content()
{
}	
function ptz_save_content()
{
}

//user
$(function(){
	$(".pp").blur(function(){
		
		var iii=0;
		for(iii=0;iii<3;iii++)
		{
			if(this==$(".pp").eq(iii).get(0)) break;
		}
		if($(this).get(0).value != $(".p").eq(iii).get(0).value )
		{
			$(this).get(0).value="";
			$(this).get(0).focus();
			alert("Confirm password is different from password, Please retry!");
		}
	});
});

//devinfo
function devinfo_load_content(bflag)
{
	
	
	
	
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="' + g_usr + '" password="' + g_pwd + '">';
	xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
	xmlstr += '<info device_name="" device_model="" device_soc="" device_sn="" sensor_type="" hardware_version="" software_version="" build_date="" build_time="" />';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/NetSDK/System/deviceInfo" + Math.random(),
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data);
			var dvr_data = xml2json.parser(data.xml, "", false);
			$("#juan\\#devinfo\\@name").val(dvr_data.juan.conf.info.device_name);
			$("#juan\\#devinfo\\@model").val(dvr_data.juan.conf.info.device_model);
			$("#juan\\#devinfo\\@hwver").val(dvr_data.juan.conf.info.hardware_version);
			$("#juan\\#devinfo\\@swver").val(dvr_data.juan.conf.info.software_version);
			$("#juan\\#devinfo\\@reldatetime").val(dvr_data.juan.conf.info.build_date + " " + dvr_data.juan.conf.info.build_time);
			$("#juan\\#devinfo\\@alarmnum").val(dvr_data.juan.conf.spec.io_alarm);
			$("#juan\\#devinfo\\@sdnum").val(dvr_data.juan.conf.spec.sd_card);
			//$("#juan\\#devinfo\\@name")[0].value = dvr_data.juan.conf.info.device_name;
			//$("#juan\\#devinfo\\@model")[0].value = dvr_data.juan.conf.info.device_model;
			//$("#juan\\#devinfo\\@hwver")[0].value = dvr_data.juan.conf.info.hardware_version;
			//$("#juan\\#devinfo\\@swver")[0].value = dvr_data.juan.conf.info.software_version;
			//$("#juan\\#devinfo\\@reldatetime")[0].value = dvr_data.juan.conf.info.build_date + " " + dvr_data.juan.conf.info.build_time;
			//$("#juan\\#devinfo\\@alarmnum")[0].value = dvr_data.juan.conf.spec.io_alarm;
			//$("#juan\\#devinfo\\@sdnum")[0].value = dvr_data.juan.conf.spec.sd_card;				
			//alert("current model:"+dvr_data.juan.conf.info.device_model);
			if(bflag==false){
				//alert(dvr_data.juan.conf.info.device_soc);
				encode_load_content(dvr_data.juan.conf.info.device_soc, dvr_data.juan.conf.info.sensor_type);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error:" + textStatus);
		}
	});	
}

var user_management_target = "";
function user_management_prepare_rm()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	if(confirm(langstr.delete_confirm))
	{
		user_management_save_del_usr();
	}
}
function user_management_save_del_usr()
{
//	show_loading("save_del_usr()");

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<del_user name=\"" + user_management_target + "\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/del_user.xml?r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.delete_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_save_edit_usr()
{
	//alert(document.getElementById('permIt_admin').checked);
	var permit_admin = "no";
	var permit_setting = "no";
	var permit_playback = "no";

	if($('#permit_admin_' + user_management_target).is(':checked') == true){
		permit_admin = "yes";
	}
	if($('#permit_setting_' + user_management_target).is(':checked') == true){
		permit_setting = "yes";
	}
	if($('#permit_playback_' + user_management_target).is(':checked') == true){
		permit_playback = "yes";
	}
	var xmlstr = "";
	xmlstr += "<user>";
	//xmlstr += "<edit_user name=\"" + user_management_target  + "\" " + permit_admin + "\permit_live=\"\" " + permit_setting + permit_playback +  "/>";
	xmlstr += "<edit_user name=\"" + user_management_target  + "\" admin=\"" + permit_admin + "\" permit_live=\"yes\" permit_setting=\"" + permit_setting + "\" permit_playback=\"" + permit_playback + "\" />";
	xmlstr += "</user>";
	//alert(xmlstr);
	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/edit_user.xml?r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.add_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}

function user_management_prepare_add()
{
	$("#tbl_add_user")[0].style.display = "block";
	$("#tbl_modify_pwd")[0].style.display = "none";
}


function user_management_save_new_usr()
{
	var use = document.getElementById("txt_new_usr").value;
	if(use==null || use=="")
	{
		alert(langstr.warning);
		document.getElementById('username').focus();

		return false;
		}	
	
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_target = $("#txt_new_usr")[0].value;
	//alert(user_management_target);
	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<add_user name=\"" + $("#txt_new_usr")[0].value + "\" password=\"" + $("#txt_new_pwd")[0].value + "\" admin=\"\" premit_live=\"yes\" premit_setting=\"\" premit_playback=\"\" />";
	xmlstr += "</user>";
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/add_user.xml?r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.add_fail);	
			}
			else
			{
				user_management_load_content();
			}
//			user_management_data2ui(dvr_data);


			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_modify()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "block";
}
function user_management_prepare_save_modify_usr()
{
	if($("#txt_old_pwd")[0].value != g_pwd)
	{
		alert(langstr.old_pwd_wrong);
		return;
	}
	if($("#txt_modify_pwd")[0].value != $("#txt_repeat_pwd")[0].value)
	{
		alert(langstr.confirm_pwd_wrong);
		return;
	}

	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";

	user_management_dvr_target = g_usr;

	var xmlstr = "";
	xmlstr += "<user>";
	xmlstr += "<set_pass old_pass=\"" + $("#txt_old_pwd")[0].value + "\" new_pass=\"" + $("#txt_modify_pwd")[0].value + "\" />";
	xmlstr += "</user>";

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/set_pass.xml?r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd + "&content=" + xmlstr, 
		async:true,

		beforeSend: function(XMLHttpRequest){
//			alert("beforeSend");
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data.xml);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.modify_pwd_fail);	
			}
			else{
				alert(langstr.modify_success);	
				self.location.replace("index.html");	
			}

			user_management_target = "";
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
function user_management_prepare_cancel()
{
	$("#tbl_add_user")[0].style.display = "none";
	$("#tbl_modify_pwd")[0].style.display = "none";
	
	user_management_target = "";
}

function user_management_data2ui(dvr_data)
{
	var user_count = dvr_data.user.user_list.count;
	var tbl = $("#tbl_user_manage")[0];
	//alert(tbl.rows.length);
	for(var i = tbl.rows.length - 1; i >= 1; i--)
	{
		tbl.deleteRow(i);
	}

	for(var i = 0; i < user_count; i++)
	{
		var tr = tbl.insertRow(tbl.rows.length);
		var td;
		var str;
		td = tr.insertCell(tr.cells.length);
		td.innerHTML = eval("dvr_data.user.user_list.user" + i + ".name");
		
		td = tr.insertCell(tr.cells.length);
		var permit_admin = "";
//		var permit_live = "";
		var permit_setting = "";
		var permit_playback = "";
		var permit_admin_disable = "";
		
		if(eval("dvr_data.user.user_list.user" + i + ".admin") == "yes")
		{
			permit_admin = "checked";
		}else{
			permit_admin_disable="disabled";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".permit_live") == "yes")
//		{
//			permit_live = "checked";
//		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_setting") == "yes")
		{
			permit_setting = "checked";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".permit_playback") == "yes")
		{
			permit_playback = "checked";
		}
		// str = "";
  //                   str += "<input type=\"checkbox\" " + permit_admin_disable +" id=\"permit_admin_" + eval("dvr_data.user.user_list.user" + i + ".name")  + "\" " + permit_admin + "/>"+(langstr.permit_admin)+"";
  //           //		str += "<input type=\"checkbox\" id=\"permit_live\" " + permit_live + ">permit_live";
  //                   str += "<input type=\"checkbox\" id=\"permit_setting_" + eval("dvr_data.user.user_list.user" + i + ".name")  + "\" " + permit_setting + "/>"+(langstr.permit_setting)+"";
  //                   str += "<input type=\"checkbox\" id=\"permit_playback_" + eval("dvr_data.user.user_list.user" + i + ".name")  + "\" " + permit_playback + "/>"+(langstr.playback)+"";
  //                   td.innerHTML = str;
		
		// td = tr.insertCell(tr.cells.length);
		var edit_user = "";
		var del_user = "";
//		var set_pass = "";
//		edit_user = "disabled"
		if(eval("dvr_data.user.user_list.user" + i + ".edit_user") == "no")
		{
			edit_user = "disabled";
		}
		if(eval("dvr_data.user.user_list.user" + i + ".del_user") == "no")
		{
			del_user = "disabled";
		}
//		if(eval("dvr_data.user.user_list.user" + i + ".set_pass") == "no")
//		{
//			set_pass = "disabled";
//		}
// 		str = "";
// 		str += "<button type=\"button\" id=\"edit_user\" " + edit_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_save_edit_usr()\">"+(langstr.save)+"</button>";
// 		str += "<button type=\"button\" id=\"del_user\" " + del_user + " onclick=\"user_management_target='" + eval("dvr_data.user.user_list.user" + i + ".name") + "';user_management_prepare_rm()\">"+(langstr.del_user)+"</button>";
// //		str += "<input type=\"button\" id=\"set_pass\" value=\"设置\" " + set_pass + ">";

// 		td.innerHTML = str;
	}
}

function user_management_load_content()
{
//	var xmlstr = '';
//	xmlstr += '<juan ver="" seq="">';
//	xmlstr += '<conf type="read" user="admin" password="">';
//	xmlstr += '<spec vin="" ain="" io_sensor="" io_alarm="" hdd="" sd_card="" />';
//	xmlstr += '<info device_name="" device_model="" device_sn="" hardware_version="" software_version="" build_date="" build_time="" />';
//	xmlstr += '</conf>';
//	xmlstr += '</juan>';

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/user/user_list.xml?r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "username=" + g_usr + "&password=" + g_pwd, 
		async:true,
//		dataType: 'get',
//		jsonp: 'jsoncallback',
		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus, xmlhttp){
//			alert("recv:" + data);
//			alert(xmlhttp.responseText);
			var dvr_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(dvr_data.user.ret != "success")
			{
				alert(langstr.login_refresh);	
			}
			else
			{
				user_management_data2ui(dvr_data);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
//			alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			alert("error:" + textStatus);
		}
	});	
}
//playback
function playback_load_content()
{
$("#begin").timepicker({
		timeFormat: "HH:mm:00",
		showTime: false,
		showButtonPanel: false,
		timeOnlyTitle: '开始时间',//lang["playback_choose_time"],
		hourText:'小时',// lang["playback_hour"],
		minuteText: '分钟'//lang["playback_minute"]
    });
	$("#end").timepicker({
		timeFormat: "HH:mm:00",
		showTime: false,
		showButtonPanel: false,
		timeOnlyTitle: '结束时间',//lang["playback_choose_time"],
		hourText: '小时',//lang["playback_hour"],
		minuteText: '分钟'//lang["playback_minute"]
    });
}
function format_sd()
{
	//$('#format_message').html('准备开始格式化...');
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:"POST",
			url:dvr_url + '/netsdk/sdcard/format?r=' + Math.random(),
			dataType:"json",
			//jsonp: 'jsoncallback',
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			},
			success:function(data){
					$('#format_message').html(langstr.formatting);
					//setInterval("mm('/scripts/abc.asp')", 10000)
					setTimeout("status_sd()",2000); 
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});		
}
function status_sd()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/sdcard/status?r=' + Math.random(),
			dataType:"json",
			//jsonp: 'jsoncallback',
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			req .setRequestHeader('If-Modified-Since','0');
			req .setRequestHeader('Cache-Control','no-cache');
			},
			success:function(data){
				$('#format_message').html('');
				switch(data)
				{
					case 'work':$('#format_message').html(langstr.work);break;
					case 'ejected':$('#format_message').html(langstr.ejected);break;
					case 'fserror':$('#format_message').html(langstr.fserror);break;
					case 'formatting':$('#format_message').html(langstr.formatting);break;
					case 'zombie':$('#format_message').html(langstr.zombie);break;
					default:break;
				}
			},
			error:function(a,b,c){ 
				if(a.status == 401){
					alert(langstr.setting_permit);
				}else{
					alert(langstr.setting_wait);	
				}
				setTimeout("hideInfo()",500);
			}
		});		
}
function date_LongToFormat(date)
{
	var myDate = new Date(date*1000);
	yy=myDate.getFullYear().toString();
	yy=(yy.length==1)?("0"+yy):yy
	mm=(myDate.getMonth()+parseInt(1)).toString();
	mm=(mm.length==1)?("0"+mm):mm
	dd=myDate.getDate().toString();
	dd=(dd.length==1)?("0"+dd):dd
	hh=myDate.getHours().toString();
	hh=(hh.length==1)?("0"+hh):hh
	mi=myDate.getMinutes().toString();
	mi=(mi.length==1)?("0"+mi):mi
	ss=myDate.getSeconds().toString();
	ss=(ss.length==1)?("0"+ss):ss
	var myFormatDate = yy + "-" + mm + "-" + dd + "  " + hh + ":" + mi + ":" + ss;
	return myFormatDate;
}

function rec_search2()			
{
	date_LongToFormat(2);
	$('#tbl_main tr').remove();
	var date = $('.m_date')[0].value;
	var begin_time = $('#begin')[0].value;
	var end_time = $('#end')[0].value;
	var r = date.match(/^(\d{1,4})(-|\/)(\d{2})\2(\d{2})$/);
	if(r==null)
	{
		date = date.substr(6,4) + '-' + date.substr(0,2) + '-' + date.substr(3,2);
	}
	/*var beginUTC = (new Date(date).getTime()/1000)+(begin_time.substr(0,2)-8)*60*60+begin_time.substr(3,2)*60+begin_time.substr(6,2)*1;
	var endUTC = (new Date(date).getTime()/1000)+(end_time.substr(0,2)-8)*60*60+end_time.substr(3,2)*60+end_time.substr(6,2)*1;*/
	var beginUTC = (Date.UTC(date.split('-')[0],parseInt(date.split('-')[1])-1,date.split('-')[2],parseInt(begin_time.split(':')[0])-8,begin_time.split(':')[1],begin_time.split(':')[2]))/1000;
	var endUTC = (Date.UTC(date.split('-')[0],parseInt(date.split('-')[1])-1,date.split('-')[2],parseInt(end_time.split(':')[0])-8,end_time.split(':')[1],end_time.split(':')[2]))/1000;
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	$.ajax({
			type:"GET",
			url:dvr_url + '/netsdk/sdcard/media/search?beginUTC='+ beginUTC +'&endUTC='+ endUTC+'&r=' + Math.random(),
			dataType:"json",
			//jsonp: 'jsoncallback',
			beforeSend : function(req){ 
			req .setRequestHeader('Authorization', auth);
			},
			success:function(data){
				$('#tbl_main tr').remove();
				var	add0 = $('<tr><td style="width:65px;">'+(langstr.SessionID)+'</td><td style="width:55px;">'+(langstr.Channel_playback)+'</td><td style="width:138px;">'+(langstr.begin_time)+'</td><td style="width:138px;">'+(langstr.end_time)+'</td><td style="width:32px;">'+(langstr.playbacl_type)+'</td><td style="width:60px;">&nbsp;</td></tr>').appendTo($('#tbl_main'));
				for(i in data){
					var	add = $('<tr><td style="width:65px;">'+ data[i].sessionID +'</td><td style="width:55px;">'+ data[i].channelID +'</td><td style="width:138px;">'+ date_LongToFormat(data[i].beginUTC) +'</td><td style="width:138px;">'+ date_LongToFormat(data[i].endUTC) +'</td><td style="width:32px;">'+ data[i].type +'</td><td style="width:60px;"><button class="playback_play" id="'+data[i].sessionID+'" value="'+data[i].sessionID+'">'+(langstr.playback)+'</button></td></tr>').appendTo($('#tbl_main'));
				goPage(1,5);
				//$('#text_view')[0].innerHTML = data[i].sessionID;
				//alert(data[0]);
				}
				$('.playback_play').each(function(index){
					$(this).click(function(){
						//alert($(this)[0].value);
					var session_id = $(this)[0].id;
					var flv_url = 'http://'+tmp_ip+'/netsdk/sdcard/media/playbackFLV?sessionID=' + session_id+'&r=' + Math.random();
						swfobject.removeSWF("flashcontent");
						var c = document.getElementById("flashcontent");
						if (!c)
						{
							var d = document.createElement("div");
							d.setAttribute("id", "flashcontent");
							document.getElementById("flashcontent-container").appendChild(d);
						}
				
						var flashvars =
						{
							url: flv_url,
							volume: "100",
							allowFullScreen: "true",
							showFullScreenButton: "true",
							autoPlay: "true",
							autoRepeat: "true"
						};
						var params =
						{
							allowFullScreen: "true"
						};
						var attributes =
						{
							id: "flashcontent",
							name: "flashcontent"
						};
			
						swfobject.embedSWF("toobplayer_dark.swf?rnd=" + Math.round(Math.random()*10000), "flashcontent", "352", "288", "9.0.0","expressInstall.swf", flashvars, params, attributes);
					})
				})
			},
			error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.empty);
					}
					setTimeout("hideInfo()",500);
			}
		});
}
//Record
function record_load_content()
{
	status_sd();
	//SelectWeekDayTable(0);//预录像（星期一为加载默认）
}
function SelectWeekDayTable(iSet)
{ 
	for(var i = 0; i <= 7; i++)
		{
			if(i == iSet)
			{
				$('#RecordList'+ iSet).prop('checked',true);
				$('#RecordList'+ iSet).prop('disabled',true);
			}else
			{
				$('#RecordList'+ i).prop('checked',false);
				$('#RecordList'+ i).prop('disabled',false);
				$('#alldaylist').prop('checked',false);
			}
			copytoall(iSet);
		}
}
function copytoall(iSet)
{
	$('#alldaylist')[0].onclick = function(){
		for(var i = 0; i <= 7; i++)
		{
			if(i !== iSet)
			{
				if($("#alldaylist")[0].checked == true){
					$('#RecordList'+ i).prop('checked',true);
				}else{
					$('#RecordList'+ i).prop('checked',false);
				}
				document.getElementById("RecordList" + iSet).checked = true;
			}
		}
	}
}
function showtimeInputWindow(i)
{
  if(!$("#WholeDayRecord").prop("checked"))
  {
	if(i < 8)
	{
		document.getElementById('timeInputWindow').style.left = "104px";
		document.getElementById('timeInputWindow').style.top = (144 + i * 32) + "px";
	}
	else
	{
		document.getElementById('timeInputWindow').style.left = "262px";
		document.getElementById('timeInputWindow').style.top = (144 + (i - 8) * 32) + "px";			
	}	
    if(document.getElementById('timeInputWindow').style.display != "block")
	{	
		if(i < 8)
	    {
		    document.getElementById("InputTimeH").value = document.getElementById("StartTime" + (i+1) + "H").innerHTML;
		    document.getElementById("InputTimeM").value = document.getElementById("StartTime" + (i+1) + "M").innerHTML;		    
	    }
	    else
	    {
		    document.getElementById("InputTimeH").value = document.getElementById("StopTime" + (i-7) + "H").innerHTML;
		    document.getElementById("InputTimeM").value = document.getElementById("StopTime" + (i-7) + "M").innerHTML;	
	    }		
		document.getElementById('timeInputWindow').style.display = "block";
	}
	else
	{
		var szTimeH = document.getElementById("InputTimeH").value;
		var szTimeM = document.getElementById("InputTimeM").value;
		if(szTimeH.length < 2)
		{
			szTimeH = '0' + szTimeH;
		}
		if(szTimeM.length < 2)
		{
		    szTimeM = '0' + szTimeM;	
		}
		if(i == current)
		{		  
		    if(i < 8)
	        {
		        document.getElementById("StartTime" + (i+1) + "H").innerHTML = szTimeH;
		        document.getElementById("StartTime" + (i+1) + "M").innerHTML = szTimeM;		    
	        }
	        else
	        {
		        document.getElementById("StopTime" + (i-7) + "H").innerHTML = szTimeH;
		        document.getElementById("StopTime" + (i-7) + "M").innerHTML = szTimeM;	
	        }				
			document.getElementById('timeInputWindow').style.display = "none";	
		}
		else
		{			
		    if(current < 8)
	        {
		        document.getElementById("StartTime" + (current+1) + "H").innerHTML = szTimeH;
		        document.getElementById("StartTime" + (current+1) + "M").innerHTML = szTimeM;
	        }
	        else
	        {
		        document.getElementById("StopTime" + (current-7) + "H").innerHTML = szTimeH;
		        document.getElementById("StopTime" + (current-7) + "M").innerHTML = szTimeM;		
	        }
			
			if(i < 8)
	        {
		        document.getElementById("InputTimeH").value = document.getElementById("StartTime" + (i+1) + "H").innerHTML;
		        document.getElementById("InputTimeM").value = document.getElementById("StartTime" + (i+1) + "M").innerHTML;		    
	        }
	        else
	        {
		        document.getElementById("InputTimeH").value = document.getElementById("StopTime" + (i-7) + "H").innerHTML;
		        document.getElementById("InputTimeM").value = document.getElementById("StopTime" + (i-7) + "M").innerHTML;	
	        }				
			document.getElementById('timeInputWindow').style.display = "block";
		}
	}
	current = i;
  }
}

function record_type()
{   
	if($("#WholeDayRecord")[0].checked ==true)  //选中全天录像
	{
		$("#WholeDayRecordType").prop("disabled",false);
		//$("#WholeDayRecordType").val(g_arrWeekDayRecordType[m_iCurWeekDay * 4]);
		
		document.getElementById("StartTime1H").innerHTML = "00"; 
		document.getElementById("StartTime1M").innerHTML = "00";
		document.getElementById("StopTime1H").innerHTML = "24";
		document.getElementById("StopTime1M").innerHTML = "00";
		for(var j = 2; j <= 4; j++)
		{
			document.getElementById("StartTime" + j + "H").innerHTML = "00"; 
			document.getElementById("StartTime" + j + "M").innerHTML = "00";
			document.getElementById("StopTime" + j + "H").innerHTML = "00";
			document.getElementById("StopTime" + j + "M").innerHTML = "00";
		}
		
		for(var i = 1; i <= 4; i++)
		{
			$("#StartTime" + i + 'H').prop("disabled",true);   
			$("#StartTime" + i + 'M').prop("disabled",true);
			$("#StopTime" + i + 'H').prop("disabled",true);
			$("#StopTime" + i + 'M').prop("disabled",true);
			$("#RecordType" + i).prop("disabled",true); 
		}
	}
	if($("#SectionDay")[0].checked ==true)  //选中分段录像
	{
		$("#WholeDayRecordType").prop("disabled",true);
		for(var i = 1; i <= 4; i++)
		{
			$("#StartTime" + i + 'H').prop("disabled",false);   
			$("#StartTime" + i + 'M').prop("disabled",false);
			$("#StopTime" + i + 'H').prop("disabled",false);
			$("#StopTime" + i + 'M').prop("disabled",false);
			$("#RecordType" + i).prop("disabled",false); 
		}
		for(var j = 1; j <= 4; j++)
		{
			document.getElementById("StartTime" + j + "H").innerHTML = '00'; 
			document.getElementById("StartTime" + j + "M").innerHTML = '00';
			document.getElementById("StopTime" + j + "H").innerHTML = '00';
			document.getElementById("StopTime" + j + "M").innerHTML = '00';
			$("#RecordType" + j).val(g_arrWeekDayRecordType[m_iCurWeekDay*4 + j - 1]);
		}
	}
}
//reboot
function reboot()
{
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="' + g_usr + '" password="' + g_pwd + '">';
	xmlstr += '<system operation="reboot" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j&r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',


		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus, xmlhttp){
			var ret_data = xml2json.parser(xmlhttp.responseText, "", false);
			if(ret_data.juan.setup.system.operation == '\\\"no auth\\')
			{
				alert(langstr.setting_permit);	
			}else{
				setTimeout("showInfo(langstr.reboot_refresh)",disp_delaytime_ms);
				setTimeout("hideInfo()",hide_delaytime_ms);
			}
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
				$("#result").html(lang.reboot_failed);
				$("#result").css("color","red");
		}
	});	
}

//time
var strYear,strMonth,strDate,strHour,strMin,strSen;
Date.prototype.toFormatString=function()
{
	strYear=this.getUTCFullYear().toString();
	if (strYear.length<4)
	{
		var i = 4-strYear.length;
		for (var j = 0; j < i; j++)
		{
			strYear = "0" + strYear;
		}
	}
	strMonth=(this.getUTCMonth()+parseInt(1)).toString();
	strMonth=(strMonth.length==1)?("0"+strMonth):strMonth
	strDate=this.getUTCDate().toString();
	strDate=(strDate.length==1)?("0"+strDate):strDate
	strHour=this.getUTCHours().toString();
	strHour=(strHour.length==1)?("0"+strHour):strHour
	strMin=this.getUTCMinutes().toString();
	strMin=(strMin.length==1)?("0"+strMin):strMin
	strSen=this.getUTCSeconds().toString();
	strSen=(strSen.length==1)?("0"+strSen):strSen
}
//时区
function time_zone_load()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
				type:"GET",
				url:dvr_url + '/netsdk/system/time/timeZone?r=' + Math.random(),
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){	
			//	alert(data.ntpServerDomain)
					// time_ntp_load();
					$('#time_zone').val(data);
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
			}
		});
}

function time_ntp_load()
{
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
		$.ajax({
				type:"GET",
				url:dvr_url + '/netsdk/system/time/ntp',
				dataType:"json",
				beforeSend : function(req){ 
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){	
					switch (data.ntpEnabled)
					{
						case true: $("#juan_envload\\#time\\@ntp_1")[0].checked = 1;break;
						case false: $("#juan_envload\\#time\\@ntp_0")[0].checked = 1;break;
						default:break;	
					};
					$('#ntp_server').val(data.ntpServerDomain);
					ntp_change();
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
			}
		});
}
function time_zone_save()
{
	var time_zone = $('#time_zone :selected').html()
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	
	var time_zone_data = '"'+time_zone+'"';
		$.ajax({
				type:'PUT',
				url:dvr_url + '/netsdk/system/time/timeZone?r=' + Math.random(),
				dataType:'json',
				data:time_zone_data,
				async:false,
				beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
//					time_ntp_save();
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
				}
			})
}
function time_ntp_save()
{
	if ($("#juan_envload\\#time\\@ntp_1")[0].checked == true)
	{
		ntp_s = true;
	}else ntp_s = false;
	var ntp_server = $('#ntp_server').val();
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	
	var time_ntp_data = '{ "ntpEnabled": '+ntp_s+', "ntpServerDomain": "'+ntp_server+'" }';
		$.ajax({
				type:'PUT',
				url:dvr_url + '/netsdk/system/time/ntp?r=' + Math.random(),
				dataType:'json',
				data:time_ntp_data,
				async:false,
				beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
						//setTimeout("showInfo(langstr.save_success)",disp_delaytime_ms);
						showInfo(langstr.save_success);
						setTimeout("hideInfo()",hide_delaytime_ms);
				},
				error:function(a,b,c){ 
					if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
				}
			})
}
/*function time_data2ui(dvr_data)
{
	var utc_devtime = parseInt(dvr_data.juan.setup.time.value)*1000;
	var devtime = new Date(utc_devtime);
	devtime.toFormatString();
	switch(dvr_data.juan.conf.datetime.date_separator)
	{
		case "-": $("#date_break")[0].selectedIndex = 0;break;
		case "/": $("#date_break")[0].selectedIndex = 1;break;
		case ".": $("#date_break")[0].selectedIndex = 2;break;
		default: break;
	}
	switch(dvr_data.juan.conf.datetime.date_format)
	{
		case "yyyymmdd": 
			$(".date_form")[0].selectedIndex = 0;
			break;
		case "mmddyyyy": 
			$(".date_form")[0].selectedIndex = 1;
			break;
		case "ddmmyyyy": 
			$(".date_form")[0].selectedIndex = 2;
			break;
		default: break;
	}
	//$("#time_zone")[0].value = dvr_data.juan.conf.datetime.time_zone;
	$("#daylight_time")[0].value = dvr_data.juan.conf.datetime.day_saving_time;

	switch (dvr_data.juan.conf.datetime.ntp_sync)
	{
		case "yes": $("#juan_envload\\#time\\@ntp_1")[0].checked = 1;break;
		case "no": $("#juan_envload\\#time\\@ntp_0")[0].checked = 1;break;
		default:break;	
	};
	switch (dvr_data.juan.conf.datetime.ntp_user_domain)
	{
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 0;
			break;
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 1;
			break;
		case "": 
			$("#juan_envload\\#time\\@ntp_server")[0].selectedIndex = 2;
			break;
		default: break;
	}
	ntp_change();

	showtime();
}*/

/*function time_load_content()
{
	var xmlstr = '';
	xmlstr += '<juan ver="" seq="">';
	xmlstr += '<conf type="read" user="admin" password="">';
	xmlstr += '<datetime date_format="" date_separator="" time_format="" time_zone="" day_saving_time="" ntp_sync="" ntp_user_domain=""	/>';
	xmlstr += '</conf>';
	xmlstr += '<setup type="read" user="admin" password="">';
	xmlstr += '<time value="" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j&r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			var dvr_data = xml2json.parser(data.xml, "", false);
			time_zone_load();
			time_data2ui(dvr_data);
			
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}*/

/*function time_save_content()
{
	showInfo(langstr.save_setup);
	var date_form,date_break;
	switch($(".date_form")[0].selectedIndex)
	{
		case 0: date_form = "yyyymmdd";break;
		case 1: date_form = "mmddyyyy";break;
		case 2: date_form = "ddmmyyyy";break;
		default: break;
	};
	switch($("#date_break")[0].selectedIndex)
	{
		case 0: date_break = "-";break;
		case 1: date_break = "/";break;
		case 2: date_break = ".";break;
		default: break;
	};
	var ntp_s,ntp_domain;
	if ($("#juan_envload\\#time\\@ntp_1")[0].checked == true)
	{
		ntp_s = "yes";
	}else ntp_s = "no";
	switch ($("#juan_envload\\#time\\@ntp_server")[0].selectedIndex)
	{
		case 0: 
			ntp_domain = "time.windows.com";
			break;
		case 1: 
			ntp_domain = "time.nist.gov";
			break;
		default: break;
	}

	savetime();

	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<conf type="write" user="admin" password="">';
	xmlstr += '<datetime date_format="' + date_form + '" date_separator="' + date_break + '" time_zone="' + $("#time_zone")[0].value + '" day_saving_time="' + $("#daylight_time")[0].value + '" ntp_sync="' + ntp_s + '" ntp_user_domain="' + ntp_domain + '"	/>';
	xmlstr += '</conf>';
	xmlstr += '</juan>';
//	alert(xmlstr);

	dvr_ajax = $.ajax({ 
		type:"GET",
		url: dvr_url + "/cgi-bin/gw2.cgi?f=j&r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "xml=" + xmlstr, 
		async:true,
		dataType: 'jsonp',
		jsonp: 'jsoncallback',

		beforeSend: function(XMLHttpRequest){
		},
		success: function(data, textStatus){
//			alert("recv:" + data.xml);
			//time_zone_save();
			setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
			setTimeout("hideInfo()",hide_delaytime_ms);
		},
		complete: function(XMLHttpRequest, textStatus){
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
		}
	});	
}*/
function showtime()
{  
	var time_show = "";
	var datebreak = $("#date_break")[0].value;
	switch ($(".date_form")[0].selectedIndex)
	{
		case 0:
			time_show = strYear+datebreak+strMonth+datebreak+strDate+"  ";
			break;
		case 1:
			time_show = strMonth+datebreak+strDate+datebreak+strYear+"  ";
			break;
		case 2:
			time_show = strDate+datebreak+strMonth+datebreak+strYear+"  ";
			break;
		default:
			break;
	}
	time_show += strHour+":"+strMin+":"+strSen;
	$("#curent_time")[0].value = time_show;
}
function savetime()
{
	//showInfo(langstr.sync_time_now);
	var currentset_date = new Date();
	currentset_date.setFullYear(parseInt(strYear, 10));
	currentset_date.setMonth(parseInt(strMonth, 10)-1);
	currentset_date.setDate(parseInt(strDate, 10));
	currentset_date.setHours(parseInt(strHour, 10));
	currentset_date.setMinutes(parseInt(strMin, 10));
	currentset_date.setSeconds(parseInt(strSen, 10));
	var currentset_utc = currentset_date.getTime()/1000;
// + currentset_date.getTimezoneOffset()*60;
//	alert(currentset_utc);
    var time_zone = $('#time_zone :selected').html()
	var xmlstr = '';
	xmlstr += '<juan ver="1.0" seq="0">';
	xmlstr += '<setup type="write" user="' + g_usr + '" password="' + g_pwd + '">';
	xmlstr += '<time value="' + currentset_utc +time_zone+ '" />';
	xmlstr += '</setup>';
	xmlstr += '</juan>';
	//alert(xmlstr);

//	dvr_ajax = $.ajax({ 
//		type:"GET",
//		url: dvr_url + "/NetSDK/S?f=j&r=" + Math.random(), 
//		processData: false, 
//		cache: false,
//		data: "xml=" + xmlstr, 
//		async:true,
//		dataType: 'jsonp',
//		jsonp: 'jsoncallback',
//
//		beforeSend: function(XMLHttpRequest){

//		},
//		success: function(data, textStatus, xmlhttp){
//			//alert(data.xml);
//			//setTimeout("showInfo(langstr.sync_refresh)",disp_delaytime_ms);
//			//setTimeout("hideInfo()",hide_delaytime_ms);
//			var ret_data = xml2json.parser(data.xml, "", false);
//			if(ret_data.juan.setup.time.value == "no auth")
//			{
//				alert(langstr.setting_permit);	
//			}
//		},
//		complete: function(XMLHttpRequest, textStatus){
//		},
//		error: function(XMLHttpRequest, textStatus, errorThrown){
//		}
//	});	
//	
	//var time_zone = $('#time_zone :selected').html()
	var auth = "Basic " + base64.encode(g_usr+':'+g_pwd);
	//var auth = "Basic " + base64.encode('admin:');
	var iso8063 = '\"' + strYear + '-' + strMonth + '-' + strDate + 'T' + strHour + ':' + strMin + ':' + strSen + '+08:00' +'\"';
		$.ajax({
				type:'PUT',
				url:dvr_url + '/netsdk/system/time/localTime?r=' + Math.random(),
				dataType:'json',
				data:iso8063,
				async:false,
				beforeSend : function(req ) {
				req .setRequestHeader('Authorization', auth);
				},
				success:function(data){
					
//					time_ntp_save();
//				alert(iso8063);
//					alert('成功');

				},
				error:function(a,b,c){ 

/// FIXME
if(a.status == 200) {

} else if(a.status == 401){
						alert(langstr.setting_permit);
					}else{
						alert(langstr.setting_wait);	
					}
					setTimeout("hideInfo()",500);
				}
			})
}
function sync_pc_time()
{
	strYear = yy;
	strMonth = mm;
	strDate = dd;
	strHour = hh;
	strMin = mi;
	strSen = ss;
	showtime();
	savetime();
}

function manual_set_time()
{
	var date_sep,time_sep;
	date_sep = $(".m_date")[0].value.split("-");
	time_sep = $('.in');//$("#m_time")[0].value.split(":");
	strYear = date_sep[0]; strHour = $('.in').eq(0).val();
	strMonth = date_sep[1]; strMin = $('.in').eq(1).val();
	strDate = date_sep[2]; strSen = $('.in').eq(2).val();
	showtime();
	savetime();
}

function is_valid_zone()
{
	var obj_zone=document.getElementById('time_zone');
	var str=obj_zone.value;
	var a = str.match(/^-?[1-9]$|^-?1[1-2]$|^0$/);
	if (a == null) 
	{
		alert('时区范围：\n     [-12~12]');
		obj_zone.value="8";
		return false;
	}
	return true;
}

function is_valid_daylight()
{
	var obj_daylight=document.getElementById('daylight_time');
	var str=obj_daylight.value;
	var a = str.match(/^-?[1-3]$|^0$/);
	if (a == null) 
	{
		alert('夏令时范围：\n     [-3~3]');
		obj_daylight.value="0";
		return false;
	}
	return true;
}

function is_valid_time()
{
	obj_time=document.getElementById('m_time');
	var str=obj_time.value;
	var a = str.match(/^(\d{2})(:)?(\d{2})\2(\d{2})$/);
	if (a == null) 
	{
		alert(langstr.format_wrong+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	if (a[1]>24 || a[3]>60 || a[4]>60)
	{
		alert(langstr.format_wrong+'13:23:05');
		obj_time.value="00:00:00";
		return false;
	}
	return true;
}

function is_valid_date()
{
	obj_date=document.getElementById('m_date');
	var str=obj_date.value;
	var r = str.match(/^(\d{1,4})(-|\/)(\d{2})\2(\d{2})$/);
	if(r==null)
	{
		alert(langstr.format_wrong+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	var d= new Date(r[1], r[3]-1, r[4]);
	if ((d.getFullYear()==r[1]&&(d.getMonth()+1)==r[3]&&d.getDate()==r[4])==false)
	{
		alert(langstr.format_wrong+"2012-01-01");
		obj_date.value="0000-00-00";
		return false;
	}
	return true;
}

// function ntp_change()
// {
// 	if ($("#juan_envload\\#time\\@ntp_1")[0].checked == 1)
// 	{
// 		$("#ntp_server")[0].disabled = false;
// 	}else
// 	{
// 		$("#ntp_server")[0].disabled = true;
// 	}
// }

//default_setting
function default_setting()
{
	// var xmlstr = '';
	// xmlstr += '<juan ver="1.0" seq="0">';
	// xmlstr += '<setup type="write" user="' + g_usr + '" password="' + g_pwd + '">';
	// xmlstr += '<system operation="default factory" />';
	// xmlstr += '</setup>';
	// xmlstr += '</juan>';
// //	alert(xmlstr);

	NetSDK_User(g_usr, g_pwd);
	NetSDK_PutJSON("/NetSDK/System/operation/default", "", function(Obj){
		// setTimeout("showInfo(langstr.save_refresh)",disp_delaytime_ms);
		// setTimeout("hideInfo()",hide_delaytime_ms);
		alert("Success");
	});

}




var upload_persent = 0;
$(function(){
	$('#swfupload-control').swfupload({
		upload_url: "/cgi-bin/upload.cgi",
		file_size_limit : "16384",
		file_types : "*.rom",
		file_types_description : "Upgrade File",
		file_upload_limit : "0",
		flash_url : "images/swfupload.swf",
		button_image_url : 'images/upload_img.png',
		button_width : 61,
		button_height : 22,
//		button_text_top_padding : 5,
//		button_text_left_padding : 5,
		button_text_style : "font-size: 22pt;",
		button_text : langstr.upgrade,
		button_disabled : false,
		button_placeholder : $('#button')[0],
		debug: false,
		custom_settings : {something : "here"}
	})
	.bind('swfuploadLoaded', function(event){
		//$('#log').append('<li>Loaded</li>');
		$('#txt_status')[0].innerHTML = "";
	})
	.bind('fileQueued', function(event, file){
		//$('#log').append('<li>File queued - '+file.name+'</li>');
		// start the upload since it's queued
		$(this).swfupload('startUpload');
	})
	.bind('fileQueueError', function(event, file, errorCode, message){
		//$('#log').append('<li>File queue error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = langstr.file_error+ message;
		$('#txt_progress')[0].innerHTML = "";
	})
	.bind('fileDialogStart', function(event){
		//$('#log').append('<li>File dialog start</li>');
	})
	.bind('fileDialogComplete', function(event, numFilesSelected, numFilesQueued){
		//$('#log').append('<li>File dialog complete</li>');
	})
	.bind('uploadStart', function(event, file){
		//$('#log').append('<li>Upload start - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = langstr.start_upload;
		$(this).swfupload('setButtonDisabled', true);
	})
	.bind('uploadProgress', function(event, file, bytesLoaded){
		//$('#log').append('<li>Upload progress - '+bytesLoaded+'</li>');
		var str = "";
		var persent = bytesLoaded/file.size*100 + "";
		upload_persent = bytesLoaded/file.size*100;
		for(var i = 0; i < bytesLoaded/file.size*10; i++){
			str += "|";
		}
		if(upload_persent < 100){
			persent = persent.substr(0, 2);
		}
		if(upload_persent >= 100){
			persent = persent.substr(0, 3);
		}
		str += persent + "%";
		$('#txt_progress')[0].innerHTML = str;
	})
	.bind('uploadSuccess', function(event, file, serverData){
		//$('#log').append('<li>Upload success - '+file.name+'</li>');
		$('#txt_status')[0].innerHTML = langstr.stop_upload;
	})
	.bind('uploadComplete', function(event, file){
		//$('#log').append('<li>Upload complete - '+file.name+'</li>');
		if(upload_persent >= 100)
		{
			$('#txt_status')[0].innerHTML = langstr.stop_upload;
			get_upgrade_rate();
		}
		else
		{
			$('#txt_status')[0].innerHTML = langstr.wait_reboot;
		}
		// upload has completed, lets try the next one in the queue
		$(this).swfupload('startUpload');
	})
	.bind('uploadError', function(event, file, errorCode, message){
		//$('#log').append('<li>Upload error - '+message+'</li>');
		$('#txt_status')[0].innerHTML = langstr.fail_upload;
	});
});	
var upgrade_persent = 0;
function get_upgrade_rate()
{
	$.ajax({ 
		type:"GET",
		url: "/cgi-bin/upgrade_rate.cgi?cmd=upgrade_rate&r=" + Math.random(), 
		processData: false, 
		cache: false,
		data: "", 
		async:true,
		beforeSend: function(XMLHttpRequest){
			//alert("beforeSend");
		},
		success: function(data, textStatus){
//			alert("recv:" + data);
			$('#txt_upgrade_status')[0].innerHTML = langstr.writing_firmware;
			upgrade_persent = parseInt(data);
			
			var str = "";
			var persent = upgrade_persent + "";
			for(var i = 0; i < persent/10; i++){
				str += "|";
			}
			if(persent != "100"){
				persent = persent.substr(0, 2);
			}
			str += persent + "%";
			$('#txt_upgrade_progress')[0].innerHTML = str;
		},
		complete: function(XMLHttpRequest, textStatus){
			//alert("complete:" + textStatus);
		},
		error: function(XMLHttpRequest, textStatus, errorThrown){
			//alert("error");	
		}
	});
	
	if(upgrade_persent <= 99)
	{
		setTimeout("get_upgrade_rate()", 1000);
	}
	else
	{
		alert(langstr.upgrade_success);
	}
}

//显示灰色 jQuery 遮罩层 
function showBg_dana() { 
	var bh = $("body").height(); 
	var bw = $("body").width(); 
	$("#fullbg_dana").css({ 
		height:bh, 
		width:bw, 
		display:"block" 
	}); 
	$("#dialog_dana").show(); 
} 
//关闭灰色 jQuery 遮罩 
function closeBg_dana() { 
$("#fullbg_dana,#dialog_dana").hide(); 
} 

function danale_load_content(){
		$('#dana_qrcode').find('div').remove();	
		var QRcode = $('<div  style=" background-image:url(http://'+tmp_ip+'/tmp/dana_id.bmp); width:124px; height:124px;"></div>').appendTo($('#dana_qrcode'));
}

function codeType_changed()
{
	switch($('#codecType').val())
		{
			case 'H.264':$('#h264_profile')[0].disabled=false;break;
			case 'H.265':$('#h264_profile')[0].disabled=true;break;
			case 'H.264+':$('#h264_profile')[0].disabled=false;break;
        	case 'H.265+':$('#h264_profile')[0].disabled=true;break;
		}
}
