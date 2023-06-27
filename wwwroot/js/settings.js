var is_setting_open = false;
function setNicknameCheck(parentObj = document) {
    let inputElement = parentObj.querySelector("#setting__name");
    let str = inputElement.value;
    let errorelem = parentObj.querySelector("#itemNickName .SettingMain__settingRow");
    let lengthObj = parentObj.querySelector(".SettingMain__settingItemStringLen--name span");
    let errorMsg = '';
    let strLength = getStringLength(str);
    errorelem.classList.remove("-success");
    if (0 < strLength && !str.match(/^[a-zA-Z0-9_-]+$/)) {
        errorelem.classList.add("-error");
        errorMsg = GetLocalizeMsg('e_error_code_AccountNameUnusableCharacters');
    }
    else if (0 < strLength && !str.match(/^[a-zA-Z]/)) {
        errorelem.classList.add("-error");
        errorMsg = GetLocalizeMsg('e_error_code_AccountNameFirstCharacters');
    }
    else if (strLength < PROFILE_NAME_MIN_NUM || strLength > PROFILE_NAME_LIMIT_NUM) {
        errorelem.classList.add("-error");
        errorMsg = GetLocalizeMsg('e_error_code_AccountNameOverLimit');
    }
    else {
        errorelem.classList.remove("-error");
    }
    var errormessage = parentObj.querySelector("#itemNickName .SettingMain__settingItemErrorMessage");
    errormessage.innerText = errorMsg;
    lengthObj.innerText = getStringLength(str).toString();
}
function setIntroductionLength(parentObj = document) {
    let inputElement = parentObj.querySelector("#setting__comment");
    let str = inputElement.value;
    let errorElem = parentObj.querySelector("#itemIntroduction .SettingMain__settingRow");
    let lengthObj = parentObj.querySelector(".SettingMain__settingItemStringLen--introduction span");
    let errorMsg = '';
    let strLength = getStringLength(str);
    errorElem.classList.remove("-success");
    if (strLength > PROFILE_COMMENT_LIMIT_NUM) {
        errorElem.classList.add("-error");
        errorMsg = GetLocalizeMsg('e_error_code_AccountCommentOverLimit');
    }
    else if ((str.match(/\n/g) || []).length >= 5) {
        errorElem.classList.add("-error");
        errorMsg = GetLocalizeMsg('e_error_code_NewLineOverLimit');
    }
    else {
        errorElem.classList.remove("-error");
    }
    var errormessage = parentObj.querySelector('#itemIntroduction .SettingMain__settingItemErrorMessage');
    errormessage.innerText = errorMsg;
    lengthObj.innerText = strLength.toString();
}
function ChangeSettingServer() {
    let updatetype = parseInt(this.dataset.updatetype);
    let cuid = this.dataset.cuid;
    cuid = (cuid ? cuid : null);
    console.log('cuid => ' + cuid);
    var value = null;
    var isUpdate = false;
    if (this.tagName === "INPUT" && this.type === "radio") {
        let element = document.querySelector(".SettingMain__settingItem--inputRadio input[name='" + this.name + "']:checked");
        if (element) {
            value = element.value;
        }
    }
    else if (this.tagName === "INPUT" && this.type === "checkbox") {
        value = this.checked;
        isUpdate = true;
    }
    else {
        const target = this.dataset.target;
        let targetObj = document.getElementById(target);
        value = targetObj.value;
    }
    if (isUpdate || profile_data.account[PROFILE_UPDATE_TYPE[updatetype]] !== value) {
        var object = { "update_type": updatetype, "cuid": cuid, "value": value };
        var json = JSON.stringify(object);
        console.log(json);
        ipcRenderer.send("change_profile", json);
    }
}
function BsChangeSetting() {
    const target = this.dataset.target;
    const cuid = this.dataset.cuid ? this.dataset.cuid : null;
    let target_value = null;
    if (this.tagName === "SELECT") {
        target_value = this.value;
    }
    else if (this.tagName === "INPUT") {
        console.log("type:" + this.type);
        if (this.type === "checkbox") {
            target_value = this.checked;
            if (target === 'notification_function_display') {
                setNotifyAllCheckedLabel(document, this.checked);
            }
        }
        else if (this.type === "radio") {
            let elements = document.querySelectorAll(".SettingMain__settingItem--inputRadio input[name='" + this.name + "']");
            console.log("type:" + this.type);
            elements.forEach(function (ele) {
                if (ele.checked) {
                    target_value = ele.value;
                    return true;
                }
            });
        }
    }
    else {
        console.log("TAG:" + this.tagName);
    }
    var object = { "target_name": target, "cuid": cuid, "target_value": target_value };
    var json = JSON.stringify(object);
    console.log(json);
    ipcRenderer.send("bs_change_setting", json);
}
function setNotifyAllCheckedLabel(obj, allChecked) {
    let notifyInput = obj.querySelectorAll(".SettingMain__NotificationTable .ToggleCheckBox__label:not(.all_checked_label)");
    notifyInput.forEach(function (ele) {
        ele.classList.toggle('disabled', !allChecked);
    });
}
function showUserIconSetting(open) {
    let targetObj = document.querySelector('.SettingProfileModal');
    let display = window.getComputedStyle(targetObj, '').display;
    if (open) {
        if (display === 'none') {
            setSelectedIconList();
            targetObj.style.display = "flex";
            TweenMax.to(targetObj, .5, {
                opacity: "1"
            });
            let iconListScrollBarInner = new SimpleBar(document.querySelector('.SettingProfileModal__iconListScrollBarInner'));
        }
    }
    else {
        if (display !== 'none') {
            TweenMax.to(targetObj, .5, {
                opacity: "0",
                onComplete: function () {
                    targetObj.style.display = 'none';
                }
            });
            let target = document.querySelector('.SettingProfileModal__settingRow');
            if (target) {
                target.classList.remove('-error');
                let error = target.querySelector('.SettingProfileModal__settingItemErrorMessage');
                if (!error)
                    return;
                error.innerText = '';
            }
        }
    }
}
function setSelectedIconList() {
    let avatar_id = profile_data.account['avatar_id'];
    let checkedObj = document.querySelector(`.SettingProfileModal__itemInput[data-targetid='${avatar_id}']`);
    if (checkedObj) {
        checkedObj.checked = true;
    }
}
function ShowSetting(open, tab, isInit = true) {
    if (open) {
        const optionPanel = document.querySelector('.OptionPanel');
        optionPanel.classList.remove('rollup');
        if (isInit) {
            ipcRenderer.send("get_settings", tab);
            return;
        }
        else {
            const userInfo = document.querySelector('.UserInfo');
            userInfo.classList.remove('-show');
            setCurrentSettingMenu(tab);
        }
    }
    else {
        if (!is_setting_open)
            return;
        if (selected_content_type === CONTENT_TYPE_PC) {
            ipcRenderer.send("get_content_options", selected_cuid);
        }
    }
    let settings = document.querySelector(".Setting");
    if (open) {
        settings.style.display = "flex";
        TweenMax.to(settings, .5, {
            opacity: '1'
        });
    }
    else {
        TweenMax.to(settings, .5, {
            opacity: '0',
            onComplete: function () {
                settings.style.display = "none";
            }
        });
    }
    document.body.classList.toggle("-no-scroll", open);
    is_setting_open = open;
}
function SettingTextCountDisp() {
    let parentEle = this.parentNode.parentNode;
    let target = parentEle.querySelector(".SettingMain__settingItemStringLen span");
    target.innerText = getStringLength(this.value).toString();
}
function SettingTextCountDispFirst(inputObj) {
    let parentEle = inputObj.parentNode.parentNode;
    let target = parentEle.querySelector(".SettingMain__settingItemStringLen span");
    target.innerText = getStringLength(inputObj.value).toString();
}
function setCurrentSettingMenu(dataMode) {
    if (dataMode === void 0) {
        dataMode = "user";
    }
    var showUser = document.querySelector('.SettingMain__user');
    var showOption = document.querySelector('.SettingMain__option');
    var showUserTitle = document.querySelector('.SettingSideMenu__menu--user');
    var showOptionTitle = document.querySelector('.SettingSideMenu__menu--option');
    var showUserImg = document.querySelector('.SettingSideMenu__menu--user img');
    var showOptionImg = document.querySelector('.SettingSideMenu__menu--option img');
    document.querySelectorAll('.SettingSideMenu__menu a').forEach(function (menu) {
        menu.classList.remove('-current');
    });
    if (dataMode == "user") {
        showUser.style.display = 'block';
        showOption.style.display = 'none';
        showUserImg.src = showUserImg.dataset.currentSrc;
        showOptionImg.src = showOptionImg.dataset.previousSrc;
        showUserTitle.classList.add('-current');
    }
    else {
        showUser.style.display = 'none';
        showOption.style.display = 'block';
        showUserImg.src = showUserImg.dataset.previousSrc;
        showOptionImg.src = showOptionImg.dataset.currentSrc;
        showOptionTitle.classList.add('-current');
    }
}
function setSettingProfile(targetName, targetValue) {
    if (targetName === 'name' || targetName === 'comment') {
        let targetObj = document.querySelector(`#setting__${targetName}`);
        if (targetObj) {
            targetObj.value = targetValue;
            if (targetName === 'name') {
                setNicknameCheck();
            }
            else if (targetName === 'comment') {
                setIntroductionLength();
            }
        }
    }
    else if (targetName === 'disclosure_scope' || targetName === 'community_state_disclosure') {
        let targetObjs = document.getElementsByName(targetName);
        if (targetObjs.length > 0) {
            targetObjs.forEach(function (obj) {
                if (obj.value === targetValue) {
                    obj.checked = true;
                }
            });
        }
    }
}
function selectedIcon() {
    try {
        if (profile_data) {
            let selectedIconId = profile_data.account['avatar_id'];
            let selectedIconUrl = profile_data.account['source_url'];
            if (is_setting_open) {
                let targetObj = document.querySelector('.SettingProfileModal');
                if (targetObj) {
                    let display = window.getComputedStyle(targetObj, '').display;
                    if (display === 'none') {
                        setSelectedIconList();
                    }
                }
                let settingIcon = document.querySelector(".SettingMain__ImageUser>img");
                settingIcon.src = selectedIconUrl;
            }
            let userInfoIcon = document.querySelector(".UserInfo__headUserIcon>img");
            userInfoIcon.src = selectedIconUrl;
            let headerIcon = document.querySelector(".Operation__userIcon>img");
            headerIcon.src = selectedIconUrl;
        }
    }
    catch (e) {
        console.log("[selectedIcon Error] error => " + e.message);
    }
}
function updateIconList(avatar_list) {
    let parentObj = document.querySelector('.SettingProfileModal__list');
    let cloneObj = parentObj.cloneNode(false);
    avatar_list.forEach(function (item) {
        let newObj = createEle('li', 'SettingProfileModal__item');
        newObj.dataset.avatarid = item.avatar_id;
        let label = createEle('label', 'SettingProfileModal__itemLabel ignore_rh');
        let input = createEle('input', 'SettingProfileModal__itemInput ignore_rh');
        input.type = 'radio';
        input.name = 'change_user_icon';
        input.dataset.targetid = item.avatar_id;
        input.value = item.avatar_id;
        let containerDiv = createEle('li', 'SettingProfileModal__itemContainer ignore_rh');
        let iconDiv = createEle('li', 'SettingProfileModal__userIcon');
        let iconImg = createEle('img');
        iconImg.src = item.source_url;
        newObj.appendChild(label).appendChild(containerDiv).appendChild(iconDiv).appendChild(iconImg);
        cloneObj.appendChild(newObj);
    });
    parentObj.parentNode.replaceChild(cloneObj, parentObj);
    selectedIcon();
}
function initSetting() {
    document.querySelector(".Operation__setting").addEventListener("click", function (event) {
        closeAllModal('Setting');
        ShowSetting(!is_setting_open, "option");
    }, false);
    document.querySelector(".UserInfo__headEditButton").addEventListener("click", function (event) {
        closeAllModal('Setting');
        ShowSetting(!is_setting_open, "user");
    }, false);
    try {
        ipcRenderer.on('update_avatar_list', function (event, arg) {
            console.log('update_avatar_list => ' + arg);
            var json = JSON.parse(arg);
            updateIconList(json.avatar_list);
        });
        ipcRenderer.on('update_settings', function (event, arg) {
            console.log('update_settings => ' + arg);
            var result = XMLHttp.get("/Home/Settings", { tab: arg }, 'document');
            result.then((response) => {
                let parentObj = document.querySelector(".Setting");
                if (parentObj) {
                    let settinObj = response.body.firstChild;
                    let nameInput = settinObj.querySelector("#setting__name");
                    nameInput.addEventListener("keyup", function () { setNicknameCheck(); }, false);
                    setNicknameCheck(settinObj);
                    let btnChangeName = settinObj.querySelector("#update_setting__name");
                    btnChangeName.addEventListener("click", ChangeSettingServer, false);
                    let btnCopyLauncherId = settinObj.querySelector("#copyBtn__launcherId");
                    btnCopyLauncherId.addEventListener("click", function () {
                        copyClipborad("#setting__launcherId", true);
                        let msgLauncherId = document.querySelector("#launcherId .SettingMain__settingItemErrorMessage");
                        msgLauncherId.classList.add("-copy");
                        TweenMax.to(msgLauncherId, .2, {
                            opacity: "1",
                            onComplete: function () {
                                TweenMax.to(msgLauncherId, .4, {
                                    delay: 2.0,
                                    opacity: "0",
                                    onComplete: function () {
                                        msgLauncherId.classList.remove("-copy");
                                    }
                                });
                            }
                        });
                    }, false);
                    let commentInput = settinObj.querySelector("#setting__comment");
                    commentInput.addEventListener("keyup", function () { setIntroductionLength(); }, false);
                    setIntroductionLength(settinObj);
                    let btnChangeComment = settinObj.querySelector("#update_setting__comment");
                    btnChangeComment.addEventListener("click", ChangeSettingServer, false);
                    let radioInput = settinObj.querySelectorAll(".SettingMain__settingItem--inputRadio>.RadioButton");
                    radioInput.forEach(function (ele) {
                        ele.addEventListener("change", ChangeSettingServer);
                    });
                    let dlSelect = settinObj.querySelector("#setting__download_speed");
                    dlSelect.addEventListener("change", BsChangeSetting, false);
                    let dlAutoSelect = settinObj.querySelector("#setting__download_speed_during_auto_update");
                    dlAutoSelect.addEventListener("change", BsChangeSetting, false);
                    let dlCacheBtn = settinObj.querySelector("#setting__download_cache_delete");
                    dlCacheBtn.addEventListener("click", function (e) {
                        ipcRenderer.send("download_cache_delete", "");
                    }, false);
                    let autoupdateInput = settinObj.querySelectorAll(".setting__autoupdate");
                    autoupdateInput.forEach(function (ele) {
                        ele.addEventListener("click", BsChangeSetting);
                    });
                    let notifyInput = settinObj.querySelectorAll(".SettingMain__NotificationTable .ToggleCheckBox__checkbox");
                    notifyInput.forEach(function (ele) {
                        ele.addEventListener("click", BsChangeSetting);
                    });
                    let allCheckedObj = settinObj.querySelector(".SettingMain__NotificationTable .ToggleCheckBox__checkbox[data-target='notification_function_display']");
                    setNotifyAllCheckedLabel(settinObj, allCheckedObj.checked);
                    let lanucherAutoStartInput = settinObj.querySelector("#setting__lanucher_auto_start");
                    lanucherAutoStartInput.addEventListener("click", BsChangeSetting, false);
                    let autologinInput = settinObj.querySelector("#setting__auto_login");
                    autologinInput.addEventListener("click", BsChangeSetting, false);
                    let mailAuthInput = settinObj.querySelector("#setting__mailauth");
                    if (mailAuthInput) {
                        mailAuthInput.addEventListener("change", ChangeSettingServer, false);
                    }
                    let backs = settinObj.querySelectorAll(".SettingSideMenu__back");
                    backs.forEach(function (back) {
                        back.addEventListener("click", function (event) {
                            event.preventDefault();
                            ShowSetting(false, arg);
                        }, false);
                    });
                    let iconListOpenBtn = settinObj.querySelector('.SettingMain__content');
                    iconListOpenBtn.addEventListener('click', function () {
                        showUserIconSetting(true);
                    }, false);
                    let iconSelectBtn = settinObj.querySelector('.SettingProfileModal__selectButton');
                    iconSelectBtn.addEventListener('click', function () {
                        let valueObj = document.querySelector('.SettingProfileModal__itemInput:checked');
                        let updatetype = parseInt(this.dataset.updatetype);
                        let value = valueObj.value;
                        if (profile_data.account[PROFILE_UPDATE_TYPE[updatetype]] !== value) {
                            var object = { "update_type": updatetype, "value": value };
                            var json = JSON.stringify(object);
                            console.log(json);
                            ipcRenderer.send("change_profile", json);
                        }
                    }, false);
                    let iconListCloseBtn = settinObj.querySelector('.SettingProfileModal__cancelButton');
                    iconListCloseBtn.addEventListener('click', function () {
                        showUserIconSetting(false);
                    }, false);
                    let menus = settinObj.querySelectorAll(".SettingSideMenu__menu a");
                    menus.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            var dataMode = this.dataset.mode;
                            setCurrentSettingMenu(dataMode);
                        }, false);
                    });
                    let directorys = settinObj.querySelectorAll(".DirectorySelector__button");
                    directorys.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            let targetCuid = this.parentNode.parentNode.parentNode.dataset.cuid;
                            ipcRenderer.send("content_select_directory", targetCuid);
                        }, false);
                    });
                    let uninstallBtns = settinObj.querySelectorAll(".ContentUninstall__button");
                    uninstallBtns.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            let targetCuid = this.parentNode.dataset.cuid;
                            ipcRenderer.send("send_content_uninstall", targetCuid);
                        }, false);
                    });
                    let releses = settinObj.querySelectorAll(".show_relese_note");
                    releses.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            ipcRenderer.send('show_relese_note', '');
                        });
                    });
                    parentObj.parentNode.replaceChild(settinObj, parentObj);
                    ShowSetting(true, arg, false);
                }
            }).catch((error) => {
                console.log("[CHATCH ERROR]" + error);
            });
        });
        ipcRenderer.on('update_total_settings', function (event, arg) {
            console.log('update_total_settings => ' + arg + ' is_setting_open => ' + is_setting_open);
            if (!is_setting_open)
                return;
            var result = XMLHttp.get("/Home/TotalSettings", { tab: arg }, 'document');
            result.then((response) => {
                let parentObj = document.querySelector("#_TotalSettings");
                if (parentObj) {
                    let settinObj = response.body.firstChild;
                    let dlSelect = settinObj.querySelector("#setting__download_speed");
                    dlSelect.addEventListener("change", BsChangeSetting, false);
                    let dlAutoSelect = settinObj.querySelector("#setting__download_speed_during_auto_update");
                    dlAutoSelect.addEventListener("change", BsChangeSetting, false);
                    let autoupdateInput = settinObj.querySelectorAll(".setting__autoupdate");
                    autoupdateInput.forEach(function (ele) {
                        ele.addEventListener("click", BsChangeSetting);
                    });
                    let dlCacheBtn = settinObj.querySelector("#setting__download_cache_delete");
                    dlCacheBtn.addEventListener("click", function (e) {
                        ipcRenderer.send("download_cache_delete", "");
                    }, false);
                    let lanucherAutoStartInput = settinObj.querySelector("#setting__lanucher_auto_start");
                    lanucherAutoStartInput.addEventListener("click", BsChangeSetting, false);
                    let autologinInput = settinObj.querySelector("#setting__auto_login");
                    autologinInput.addEventListener("click", BsChangeSetting, false);
                    let directorys = settinObj.querySelectorAll(".DirectorySelector__button");
                    directorys.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            let targetCuid = this.parentNode.parentNode.parentNode.dataset.cuid;
                            ipcRenderer.send("content_select_directory", targetCuid);
                        }, false);
                    });
                    let uninstallBtns = settinObj.querySelectorAll(".ContentUninstall__button");
                    uninstallBtns.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            let targetCuid = this.parentNode.dataset.cuid;
                            let locationChange = document.querySelector(".SettingMain__settingRow[data-updatetype='install_location'][data-cuid='" + targetCuid + "'] .DirectorySelector__button");
                            if (locationChange) {
                                locationChange.classList.toggle('disabled', true);
                            }
                            this.classList.toggle('disabled', true);
                            ipcRenderer.send("send_content_uninstall", targetCuid);
                        }, false);
                    });
                    let releses = settinObj.querySelectorAll(".show_relese_note");
                    releses.forEach(function (button) {
                        button.addEventListener("click", function (event) {
                            event.preventDefault();
                            ipcRenderer.send('show_relese_note', '');
                        });
                    });
                    parentObj.parentNode.replaceChild(settinObj, parentObj);
                }
            }).catch((error) => {
                console.log("[CHATCH ERROR]" + error);
            });
        });
    }
    catch (e) {
        console.log("Error => " + e.message);
        ipcRenderer.send("javascript_error_msg", e.message);
    }
}
//# sourceMappingURL=settings.js.map