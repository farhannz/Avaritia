var MaintenanceCode = 503;
var BadRequestCode = 400;
var PayloadTooLargeCode = 413;
var TimeoutSec = 10000;
class XMLHttp {
    static serializeUrl(url, data) {
        let strParams = '';
        for (let key in data) {
            if (strParams.length > 0) {
                strParams += '&';
            }
            strParams += encodeURIComponent(key) + '=' + encodeURIComponent(data[key] == null ? '' : data[key]);
        }
        if (strParams.length > 0 && url.indexOf('?') === -1) {
            return url += '?' + strParams;
        }
        return url + strParams;
    }
}
XMLHttp.get = (url, data, responsetype) => {
    return XMLHttp.request(XMLHttp.serializeUrl(url, data), {}, 'GET', 200, '', responsetype);
};
XMLHttp.post = (url, data, responsetype) => {
    return XMLHttp.request(url, { 'Content-type': 'application/json' }, 'POST', 200, JSON.stringify(data), responsetype);
};
XMLHttp.put = (url, data, responsetype) => {
    return XMLHttp.request(url, { 'Content-type': 'application/json' }, 'PUT', 200, JSON.stringify(data), responsetype);
};
XMLHttp.delete = (url, data, responsetype) => {
    return XMLHttp.request(XMLHttp.serializeUrl(url, data), {}, 'DELETE', 200, '', responsetype);
};
XMLHttp.postForm = (url, data, responsetype) => {
    return XMLHttp.requestForm(url, {}, 'POST', 200, data, responsetype);
};
XMLHttp.request = (url, headers, method, status, stringdata, responsetype) => {
    let sendData = stringdata === '' ? null : stringdata;
    return XMLHttp.requestForm(url, headers, method, 200, sendData, responsetype);
};
XMLHttp.requestForm = (url, headers, method, status, sendData, responsetype) => {
    return new Promise(((resolve, reject) => {
        const req = new XMLHttpRequest();
        req.timeout = TimeoutSec;
        req.open(method, url, true);
        for (let key in headers) {
            if (headers.hasOwnProperty(key)) {
                console.log('header key:' + key + ' value:' + headers[key]);
                req.setRequestHeader(key, headers[key]);
            }
        }
        if (responsetype !== null) {
            req.responseType = responsetype;
        }
        req.onload = () => {
            if (req.readyState === XMLHttpRequest.DONE) {
                if (req.status === status) {
                    if (responsetype === 'document') {
                        resolve(req.responseXML);
                    }
                    else if (responsetype === 'blob') {
                        resolve(req.response);
                    }
                    else if (responsetype === 'json') {
                        resolve(req.response);
                    }
                    else if (responsetype === 'arraybuffer') {
                        resolve(req.response);
                    }
                    else if (responsetype === null || responsetype === 'text') {
                        resolve(req.responseText);
                    }
                }
                else {
                    console.log('StatusCode : other - ' + req.status);
                    let errorResponse = new ErrorResponse(req.status, req.response);
                    reject(errorResponse);
                }
            }
        };
        req.onerror = () => {
            console.log('onerror => req.statusText:' + req.status + ' - ' + req.statusText);
            let errorResponse = new ErrorResponse(req.status, req.response);
            reject(errorResponse);
        };
        req.ontimeout = () => {
            console.log('ontimeout => req.statusText:' + req.status + ' - ' + req.statusText);
            let errorResponse = new ErrorResponse(req.status, req.response, true);
            reject(errorResponse);
        };
        req.send(sendData);
    }));
};
class ErrorResponse {
    constructor(statusCode, body, isTimeout = false) {
        this.statusCode = statusCode;
        this.body = body;
        this.isTimeout = isTimeout;
    }
}
function sendForm(sendUrl, formId, method, appendData) {
    const form = document.getElementById(formId);
    let formData = new FormData(form);
    for (var key in appendData) {
        formData.append(key, appendData[key]);
    }
    var object = {};
    formData.forEach((value, key) => { object[key] = value; });
    method = method.toUpperCase();
    if (method === 'GET') {
        return XMLHttp.get(sendUrl, object, null);
    }
    else if (method === 'POST') {
        return XMLHttp.post(sendUrl, object, null);
    }
    else if (method === 'PUT') {
        return XMLHttp.put(sendUrl, object, null);
    }
    else if (method === 'DALETE') {
        return XMLHttp.delete(sendUrl, object, null);
    }
    else {
        return false;
    }
}
function replaceTargetDom(target_id, response) {
    let target = document.getElementById(target_id);
    while (target.firstChild)
        target.removeChild(target.firstChild);
    if (response !== null) {
        target.appendChild(response.firstChild);
    }
}
//# sourceMappingURL=request.js.map