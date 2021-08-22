function goPage(pno,psize){

var itable = document.getElementById("tbl_main");

var num = itable.rows.length;//表格行数

var totalPage = 0;//总页数

var pageSize = psize;//每页显示行数

if((num-1)/pageSize > parseInt((num-1)/pageSize)){   

   totalPage=parseInt((num-1)/pageSize)+1;   

   }else{   

   totalPage=parseInt((num-1)/pageSize);   

   }   

var currentPage = pno;//当前页数

var startRow = (currentPage - 1) * pageSize+1;//开始显示的行   

   var endRow = currentPage * pageSize+1;//结束显示的行   

   endRow = (endRow > num)? num : endRow;

//前三行始终显示

for(i=0;i<1;i++){

var irow = itable.rows[i];

irow.style.display = "block";

}

for(var i=1;i<num;i++){

var irow = itable.rows[i];

if(i>=startRow&&i<endRow){

irow.style.display = "block";	

}else{

irow.style.display = "none";

}

}

var pageEnd = document.getElementById("pageEnd");

var tempStr = langstr.total+(num-1)+"&nbsp;&nbsp;&nbsp;&nbsp;"+currentPage+'/'+totalPage+langstr.page+'&nbsp;&nbsp;&nbsp;&nbsp;';

if(currentPage>1){

tempStr += "<a href=\"#\" onClick=\"goPage("+(currentPage-1)+","+psize+")\">"+langstr.prev+"</a>&nbsp;&nbsp;&nbsp;&nbsp;"

}else{

tempStr += langstr.prev+'&nbsp;&nbsp;&nbsp;&nbsp;';	

}

if(currentPage<totalPage){

tempStr += "<a href=\"#\" onClick=\"goPage("+(currentPage+1)+","+psize+")\">"+langstr.next+"</a>&nbsp;&nbsp;&nbsp;&nbsp;";

}else{

tempStr += langstr.next+'&nbsp;&nbsp;&nbsp;&nbsp;';	

}

if(currentPage>1){

tempStr += "<a href=\"#\" onClick=\"goPage("+(1)+","+psize+")\">"+langstr.home_page+"</a>&nbsp;&nbsp;&nbsp;&nbsp;";

}else{

tempStr += langstr.home_page+'&nbsp;&nbsp;&nbsp;&nbsp;';

}

if(currentPage<totalPage){

tempStr += "&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"#\" onClick=\"goPage("+(totalPage)+","+psize+")\">"+langstr.trailer+"</a>";

}else{

tempStr += '&nbsp;&nbsp;&nbsp;&nbsp;'+langstr.trailer+'&nbsp;&nbsp;&nbsp;&nbsp;';

}

document.getElementById("barcon").innerHTML = tempStr;

}