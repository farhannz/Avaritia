var _authCodeInputDisabled = false;
function loginPostMessageCallback(postdata) {
    if (postdata.event_type == CHANNEL.GET_LOGIN_RESULT) {
        ipcRenderer.send(CHANNEL.GET_LOGIN_RESULT, postdata.event_data);
    }
    else if (postdata.event_type == CHANNEL.GET_CONTENT_LOGIN_RESULT) {
        ipcRenderer.send(CHANNEL.GET_CONTENT_LOGIN_RESULT, postdata.event_data);
    }
    else {
        console.log("[message] other");
    }
}
function showAuthCodeConfirm(open) {
    let targetObj = document.querySelector('.CodeInput');
    let display = window.getComputedStyle(targetObj, '').display;
    if (open) {
        if (display === 'none') {
            targetObj.style.display = "block";
            TweenMax.to(targetObj, .5, {
                opacity: "1"
            });
        }
    }
    else {
        if (display !== 'none') {
            TweenMax.to(targetObj, .5, {
                opacity: "0",
                onComplete: function () {
                    targetObj.style.display = "none";
                }
            });
        }
    }
}
$(function () {
    window.addEventListener('message', function (event) {
        postMessageDecode(event, loginPostMessageCallback);
    }, false);
    const authcodeSubmitBtn = document.querySelector('.CodeInput__submitButton');
    if (authcodeSubmitBtn) {
        console.log('[set authcode_confirm]');
        function errorDisp(isError, message, inputDisabled) {
            let codeInputArea = document.querySelector('.CodeInput');
            let resultObj = document.querySelector('.authcode_confirm__result');
            let authCodeObj = document.querySelector('.CodeInput__form');
            resultObj.classList.toggle('-error', isError);
            resultObj.innerHTML = replaceBr(message.replace(/\\n/g, '\n'));
            authcodeSubmitBtn.classList.toggle('disabled', false);
            authCodeObj.value = '';
            _authCodeInputDisabled = inputDisabled;
            codeInputArea.classList.toggle('-code-over', _authCodeInputDisabled);
            authCodeObj.disabled = _authCodeInputDisabled;
            authcodeSubmitBtn.disabled = _authCodeInputDisabled;
        }
        ipcRenderer.on('show_authcode_confirm', function (event, arg) {
            console.log('show_authcode_confirm => ' + arg);
            let mail = document.querySelector('.CodeInput__email');
            mail.textContent = arg;
            showAuthCodeConfirm(true);
        });
        ipcRenderer.on('hide_authcode_confirm', function (event, arg) {
            console.log('hide_authcode_confirm => ' + arg);
            showAuthCodeConfirm(false);
            let mail = document.querySelector('.CodeInput__email');
            mail.textContent = '';
            errorDisp(false, '', false);
        });
        ipcRenderer.on('check_authcode_result', function (event, arg) {
            console.log('check_authcode_result => ' + arg);
            if (arg) {
                let json = JSON.parse(arg);
                errorDisp(!json.isSuccess, json.message, json.inputDisabled);
            }
        });
        document.querySelector('.CodeInput__form').addEventListener('focus', function () {
            let resultObj = document.querySelector('.authcode_confirm__result');
            resultObj.classList.toggle('-error', false);
            resultObj.innerText = '';
        }, false);
        authcodeSubmitBtn.addEventListener('click', function (e) {
            e.preventDefault();
            let authCodeObj = document.querySelector('.CodeInput__form');
            let authCode = authCodeObj.value;
            authcodeSubmitBtn.classList.toggle('disabled', true);
            console.log('IPC [check_authcode] authCode=' + authCode);
            ipcRenderer.send('check_authcode', authCode);
        }, false);
        document.querySelector('.CodeInput__resendButton').addEventListener('click', function (e) {
            e.preventDefault();
            ipcRenderer.send('resend_authcode_request');
        }, false);
        document.querySelector('.CodeInput__otherButton').addEventListener('click', function (e) {
            e.preventDefault();
            ipcRenderer.send('back_login');
        });
    }
});
//# sourceMappingURL=login.js.map