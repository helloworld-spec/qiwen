/**
 * JavaScript format HTML function
 *
 */
function ObjectToHTML(key, parsedJson) {

    var tbl = '<tr width="100%" >';

    if (!parsedJson) {
        return '';
    }

    if (null != key) {
        tbl += '<th style="background-color:lightgray; color=white; text-align: right; padding-right: 16" width="10%">' + key + '</th>';
        tbl += '<td style="text-align: left" width="90%">';
    }

    if(typeof(parsedJson) === 'object') {
        tbl += '<table width="100%" border="1">';
        var okey;
        for (okey in parsedJson) {
            if (parsedJson.hasOwnProperty(okey)) {
                // console.log(tbl);
                tbl += ObjectToHTML(okey, parsedJson[okey]);
            }
        }
        tbl += '</table>';
    } else {
        tbl += parsedJson;
    }

    if (null != key) {
        tbl += '</td>';
    }
    tbl += '</tr>';
    // console.log(tbl);
    return tbl;
}
