function CutStrLength(str,Ilength)  
    {  
        var tmp=0;  
        var len=0;  
        var okLen=0  
        for(var i=0;i<Ilength;i++)  
        {  
            if(str.charCodeAt(i)>255)  
                tmp+=2  
            else  
                len+=1  
            okLen+=1  
            if(tmp+len==Ilength)  
            {  
                return (str.substring(0,okLen));  
                break;  
            }  
            if(tmp+len>Ilength)  
            {  
                return (str.substring(0,okLen-1)+"");  
                break;  
            }  
        }  
    }  
    function checkFieldLength(fieldName,fieldDesc,fieldLength)  
    {  
        var str=document.getElementById(fieldName).value;  
        var theLen=0;  
        var teststr='';  
        for(i=0;i<str.length;i++)  
        {  
            teststr=str.charAt(i);  
            if(str.charCodeAt(i)>255)  
            theLen=theLen+2;  
            else  
            theLen=theLen+1;  
        }  
        //document.getElementById('showMsg').innerText=theLen;  
        if(theLen>fieldLength)  
        {  
			alert(fieldDesc);
            //document.getElementById('showMsg').innerText=fieldDesc;  
            //alert(fieldDesc+" 的字段长度超过规定长度！");  
            //document.getElementById(fieldName).focus();  
            document.getElementById(fieldName).value=CutStrLength(str,fieldLength);  
            return false;  
        }  
        else  
        {  
            return true;  
        }  
    }  