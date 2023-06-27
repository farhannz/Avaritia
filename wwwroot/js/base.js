const DEFAULT_COLOR = {
    "background_url": "",
    "background_color": "rgb(32, 37, 54)",
    "logo_up_color": "#ea5514",
    "logo_down_color": "#e60000",
    "font_color": "white"
};
var rep_hits_ctrl = false;
var rep_hits_ctrl_before_obj;
var modal_selector = "body";
var isModalShow = false;
var requestModalList = [];
function createEle(eleType, className = '') {
    let ele = document.createElement(eleType);
    if (className) {
        ele.className = className;
    }
    return ele;
}
function createSelectOption(text, value, selected, className = '') {
    console.log(`[create select option] text => ${text} value => ${value} selected => ${selected} className => ${className}`);
    let ele = createEle('option', className);
    ele.innerText = text;
    ele.value = value;
    ele.selected = selected;
    return ele;
}
function setChecked(itemSelector, isChecked) {
    let elements = document.querySelectorAll(itemSelector);
    if (elements.length > 0) {
        elements.forEach(function (ele) {
            ele.checked = isChecked;
        });
    }
}
function getDictionary(obj, mainKey) {
    let res = {};
    if (obj) {
        for (let i = 0; i < obj.length; i++) {
            if (mainKey in obj[i]) {
                res[obj[i][mainKey]] = obj[i];
            }
        }
    }
    return res;
}
function getListObject(obj, getKeyList) {
    let res = {};
    if (obj) {
        for (let i = 0; i < getKeyList.length; i++) {
            let key = getKeyList[i];
            if (key in obj) {
                res[key] = obj[key];
            }
            else {
                res[key] = null;
            }
        }
    }
    else {
        for (let i = 0; i < getKeyList.length; i++) {
            res[getKeyList[i]] = null;
        }
    }
    return res;
}
function replaceBr(str) {
    return str.replace(/\r?\n/g, '<br>');
}
function loadTargeIframetUrl(loadUrl, target) {
    const targetIfream = document.getElementById(target);
    if (targetIfream != null) {
        targetIfream.src = loadUrl;
    }
}
function createOverlay(node_selector, show, hasClose) {
    if (show) {
        let targetList = document.querySelectorAll(node_selector);
        console.log("[overlay show] node_selector => " + node_selector + " targetlist => " + targetList.length);
        targetList.forEach(function (el) {
            let existList = el.querySelector(".Contents_overlay");
            if (!existList) {
                el.classList.add("Contents_overlay_parent");
                let overlay = createEle("div", "Contents_overlay");
                if (hasClose) {
                    let closeBtn = createEle("div", `c_window_close window_${windowtype}`);
                    let closeImg = createEle("img", "");
                    closeImg.src = "/img/index/WindowControl__closeButton.png";
                    closeBtn.addEventListener('click', function (e) {
                        e.preventDefault();
                        console.log("[CLOSE]" + windowtype);
                        windowActionClose(windowtype);
                    }, false);
                    overlay.appendChild(closeBtn).appendChild(closeImg);
                }
                el.appendChild(overlay);
                el.addEventListener('click', windowModalClicked, false);
            }
        });
    }
    else {
        let targetList = document.querySelectorAll(node_selector + ".Contents_overlay_parent");
        console.log("[overlay hide] node_selector => " + node_selector + " targetlist => " + targetList.length);
        targetList.forEach(function (el) {
            el.classList.remove("Contents_overlay_parent");
            let childList = el.querySelectorAll(".Contents_overlay");
            childList.forEach(c => c.remove());
        });
    }
}
function windowModalClicked() {
    console.log('[WindowModalClicked] windowType =>' + windowtype);
    ipcRenderer.send("window_modal_clicked", windowtype);
}
function windowModal(open, windowId) {
    let hasClose = (windowtype && windowtype !== 'Dialog' && windowtype !== 'Install');
    if (open) {
        isModalShow = open;
        if (requestModalList.indexOf(windowId) === -1) {
            requestModalList.push(windowId);
        }
        createOverlay(modal_selector, open, hasClose);
    }
    else {
        if (requestModalList.indexOf(windowId) !== -1) {
            requestModalList = requestModalList.filter(function (x) {
                return x !== windowId;
            });
        }
        if (requestModalList.length === 0) {
            isModalShow = open;
            createOverlay(modal_selector, open, hasClose);
        }
    }
}
function headerColorSet(backgroundColor = DEFAULT_COLOR['background_color'], fontColor = DEFAULT_COLOR['font_color']) {
    const header = document.querySelector('.Header');
    header.style.background = backgroundColor;
    const operationItems = document.querySelectorAll('.Operation__item,.WindowControl__maxmizeButton,.WindowControl__closeButton,.WindowControl__minimizeButton');
    operationItems.forEach(function (operationItem) {
        if (operationItem.querySelector('img')) {
            if (fontColor == 'black') {
                operationItem.querySelector('img').style.filter = "brightness(0%)";
            }
            else {
                operationItem.querySelector('img').style.removeProperty('filter');
            }
        }
        if (fontColor == 'black') {
            var items = document.querySelectorAll('.Operation__item.Operation__chatIcon,.Operation__item.Operation__userName');
            items.forEach(function (item) {
                item.style.borderRight = '1px solid rgba(0,0,0,0.2)';
            });
        }
        else {
            var items = document.querySelectorAll('.Operation__item.Operation__chatIcon,.Operation__item.Operation__userName');
            items.forEach(function (item) {
                item.style.removeProperty('borderRight');
            });
        }
        operationItem.style.color = fontColor;
    });
}
function checkExtension(filename, allowExtensions) {
    var ext = getExtension(filename).toLowerCase();
    if (allowExtensions.indexOf(ext) === -1)
        return false;
    return true;
}
function getExtension(filename) {
    var pos = filename.lastIndexOf('.');
    if (pos === -1)
        return '';
    return filename.slice(pos + 1);
}
function windowActionClose(window) {
    ipcRenderer.send("act_window_close", window);
}
function windowActionMinimize(window) {
    if (isModalShow)
        return;
    ipcRenderer.send("act_window_min", window);
}
function windowActionMaximize(window) {
    if (isModalShow)
        return;
    ipcRenderer.send("act_window_max", window);
}
function changeMaxmizeIcon(resultAction, maximizeIcon, unMaximizeIcon) {
    let winCtrlMaxmizeBtns = document.querySelectorAll('.c_window_maximize');
    if (winCtrlMaxmizeBtns.length > 0) {
        winCtrlMaxmizeBtns.forEach(function (button) {
            let img = button.querySelector('img');
            if (img) {
                if (resultAction === 'maximize') {
                    img.src = maximizeIcon;
                }
                else {
                    img.src = unMaximizeIcon;
                }
            }
        });
    }
}
const SIZE_KB = 1024;
const SIZE_MB = Math.pow(SIZE_KB, 2);
const SIZE_GB = Math.pow(SIZE_KB, 3);
const SIZE_TB = Math.pow(SIZE_KB, 4);
function convertSizeString(size, dp = 0) {
    let targetSize = null;
    let unit = "Byte";
    if (size >= SIZE_TB) {
        targetSize = SIZE_TB;
        unit = "TB";
    }
    else if (size >= SIZE_GB) {
        targetSize = SIZE_GB;
        unit = "GB";
    }
    else if (size >= SIZE_MB) {
        targetSize = SIZE_MB;
        unit = "MB";
    }
    else if (size >= SIZE_KB) {
        targetSize = SIZE_KB;
        unit = "KB";
    }
    let decimalPoint = dp === 0 ? 1 : Math.pow(10, dp);
    const result = targetSize !== null ? Math.floor((size / targetSize) * decimalPoint) / decimalPoint : size;
    return result + unit;
}
const MBSIZE_GB = 1024;
const MBSIZE_TB = Math.pow(MBSIZE_GB, 2);
function convertSizeStringMb(mb, dp = 2) {
    let targetSize = null;
    let unit = "MB";
    if (mb >= MBSIZE_GB) {
        targetSize = MBSIZE_GB;
        unit = "GB";
    }
    else if (mb >= MBSIZE_TB) {
        targetSize = MBSIZE_TB;
        unit = "TB";
    }
    let decimalPoint = dp === 0 ? 1 : Math.pow(10, dp);
    const result = targetSize !== null ? Math.floor((mb / targetSize) * decimalPoint) / decimalPoint : mb;
    return result + unit;
}
function getStringLength(str) {
    var result = 0;
    for (var i = 0; i < str.length; i++) {
        var chr = str.charCodeAt(i);
        if ((chr >= 0x00 && chr < 0x81) ||
            (chr === 0xf8f0) ||
            (chr >= 0xff61 && chr < 0xffa0) ||
            (chr >= 0xf8f1 && chr < 0xf8f4)) {
            result += 1;
        }
        else {
            result += 2;
        }
    }
    return result;
}
function removeSelectOptions(target) {
    while (target.lastChild) {
        target.removeChild(target.lastChild);
    }
}
function repeatedHitsCancellation(e) {
    let target = e.srcElement;
    if (target.classList.contains("ignore_rh")) {
        return;
    }
    if (rep_hits_ctrl) {
        e.preventDefault();
        e.stopImmediatePropagation();
        console.log("click cancel:" + e.type + " tag:" + target.tagName + " before:" + rep_hits_ctrl_before_obj);
        rep_hits_ctrl_before_obj = null;
        return false;
    }
    else {
        rep_hits_ctrl = true;
        rep_hits_ctrl_before_obj = target.tagName;
        setTimeout(function () {
            rep_hits_ctrl = false;
            rep_hits_ctrl_before_obj = null;
        }, 800);
    }
}
function auxclickCancellation(e) {
    e.preventDefault();
    e.stopImmediatePropagation();
    console.log("auxclick cancel:" + e.type);
}
function copyClipborad(target, isInput = false) {
    var listener = function (e) {
        let targetStr = "";
        if (isInput) {
            var targetInput = document.querySelector(target);
            targetStr = targetInput.value;
        }
        else {
            var targetEle = document.querySelector(target);
            targetStr = targetEle.textContent;
        }
        e.clipboardData.setData("text/plain", targetStr);
        e.preventDefault();
        document.removeEventListener("copy", listener);
    };
    document.addEventListener("copy", listener);
    document.execCommand("copy");
}
function copyClipboradElement(targetEle, isInput = false) {
    var listener = function (e) {
        let targetStr = "";
        if (isInput) {
            targetStr = targetEle.value;
        }
        else {
            targetStr = targetEle.innerText;
        }
        e.clipboardData.setData("text/plain", targetStr);
        e.preventDefault();
        document.removeEventListener("copy", listener);
    };
    document.addEventListener("copy", listener);
    document.execCommand("copy");
}
function deleteUrlParam(targetUrl, list) {
    let url = new URL(targetUrl);
    let params = url.searchParams;
    list.forEach(function (key) {
        params.delete(key);
    });
    url.search = '?' + params.toString();
    console.log('[deleteUrlParam] => ' + url.toString());
    return url.toString();
}
function addUrlParam(targetUrl, list) {
    let url = new URL(targetUrl);
    let params = url.searchParams;
    list.forEach(function (item) {
        params.set(item.key, item.value);
    });
    url.search = '?' + params.toString();
    console.log('[addUrlParam] => ' + url.toString());
    return url.toString();
}
function GetLocalizeMsg(key, param1 = null, param2 = null) {
    let result = null;
    if (typeof MSG_LIST === 'undefined')
        return result;
    if (key in MSG_LIST) {
        result = MSG_LIST[key];
        result = result.replace('[[0]]', param1);
        result = result.replace('[[1]]', param2);
        result = replaceBr(result);
    }
    return result;
}
$(function () {
    window.addEventListener('click', repeatedHitsCancellation, true);
    window.addEventListener('auxclick', auxclickCancellation, true);
    ipcRenderer.on('modal_ctrl', function (event, arg) {
        console.log('modal_ctrl:' + arg);
        let obj = JSON.parse(arg);
        windowModal(obj.show, obj.windowId);
    });
});
//# sourceMappingURL=base.js.map