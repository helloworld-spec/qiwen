


/**
 * NetSDK 认证用户名称。
 */
var jqUserName = "";

/**
 * NetSDK 认证用户密码。
 */
var jqUserPass = "";


function NetSDK_User(name, pass)
{
	jqUserName = name;
	jqUserPass = pass;
}

function NetSDK_Path(path)
{
	/**
	 * Add Random Number.
	 */
	if (String(path).indexOf("?") >= 0) {
		path += "&r=" + Math.random();
	} else {
		path += "?r=" + Math.random();
	}
	
	return path;
}

function NetSDK_GetJSON(path, onSuccess)
{
	path = NetSDK_Path(path);
	
	/**
	 * Get Request.
	 */
	$.ajax({
		type: "GET",
		url: path,
		contentType: "text/json",
		async: true,
		dataType: "json",
		
		beforeSend: function(jqXHR) { 
			jqXHR.setRequestHeader("Authorization", "Basic " + base64.encode(jqUserName + ":" + jqUserPass));
		},
		
		success: onSuccess,
		
		error: function(a,b,c) {
			console.log(a.status);
			if(a.status == 401) {
				//alert(langstr.setting_permit);
			} else {
				alert(langstr.setting_wait);
			}
		}	
	});
}

function NetSDK_PutJSON(path, jsonp, onSuccess)
{
	path = NetSDK_Path(path);
	
	/**
	 * Put Request.
	 */
	$.ajax({
		type: "PUT",
		url: path,
		contentType: "text/json",
		async: true,
		dataType: "json",
		data: jsonp,
		async:false,
		
		beforeSend: function(jqXHR) { 
			jqXHR.setRequestHeader("Authorization", "Basic " + base64.encode(jqUserName + ":" + jqUserPass));
		},
		
		success: onSuccess,
		
		error: function(a,b,c) {
			if (a.status == 200) {
				onSuccess;
				
			} else if(a.status == 401) {
				//alert(langstr.setting_permit);
			} else {
				alert(langstr.setting_wait);
			}
		}	
	});
}
	


