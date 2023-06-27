const CONTENT_SELECTED_CLASS = "-current";
var CONSTEN_AREA_ID = "child-frame";
const TAB_CLASS_LIST = ["-top", "-news", "-shop", "-forum", "-movies", "-other"];
const IFRAME_TIMEOUT = 10000;
const CONTENT_ACTION_INSTALL = "Install";
const CONTENT_ACTION_UNINSTALL = "Uninstall";
const installCompleteState = ["Maintenance", "Standby", "UpdateRequired"];
const uninstallCompleteState = ["NotInstalled"];
var contentlist_pc = [];
var contentlist_browser = [];
var tablist = [];
var is_load_content = true;
var selected_content_type;
var selected_content_data = DEFAULT_COLOR;
var selected_tab_idx = 0;
var selected_tab_type = 0;
var selected_content_options = null;
var profile_data;
var is_first_load = true;
var is_setting_open = false;
var is_change_cuid = false;
var waiting_action_complete = null;
var waiting_installing = [];
var waiting_uninstalling = [];
var is_content_maintenance = false;
var before_content_state = null;
var start_pending_cuid = [];
var is_loading = true;
var load_achievement_url;
var has_support_url = false;
function homePostMessageCallback(postdata) {
    if (postdata.event_type == "browser_open") {
        ipcRenderer.send("browser_open", postdata.event_data);
    }
    else if (postdata.event_type == "ipc_send") {
        ipcRenderer.send(postdata.event_data.cannel, postdata.event_data.msg);
    }
    else {
        console.log("[message-prant] other");
    }
}
function showContentMaintenance(cuid) {
    if (selected_cuid !== cuid)
        return;
    is_content_maintenance = true;
    optionsEnabled(false);
    setInstallBtn("Maintenance", "bs_content_play", BUTTON_TEXT_MAINTENANCE);
    getNotificationVersion(true);
}
function sendContent(cuid, tabType, sendUrl = null) {
    if (selected_cuid !== cuid) {
        if (!existContent(cuid)) {
            console.log("[sendContent] cuid not exist => " + cuid);
            if (start_pending_cuid.length > 0 && start_pending_cuid[0] == cuid) {
                start_pending_cuid.splice(0, 1);
                console.log("[sendContent] delete from start_pending_cuid => " + cuid);
            }
            return;
        }
        is_load_content = true;
        changeContent(cuid);
        priority_tab_type = tabType;
        priority_url = sendUrl;
        selectContent();
    }
    else {
        let index = getTargetTabTypeIndex(tabType);
        if (index === -1)
            return;
        if (index === selected_tab_idx) {
            let loadUrl = (sendUrl ? sendUrl : tablist[index].url);
            setLoadIframeTimeout('.Web', loadUrl, true);
        }
        else {
            if (is_loading)
                return;
            is_loading = true;
            coverWrapped = true;
            is_load_content = false;
            priority_tab_type = tabType;
            priority_url = sendUrl;
            serectTab();
        }
    }
}
function clearContent() {
    const btns = document.querySelectorAll("#btn_content_install,.MainMenu__button");
    btns.forEach(function (btn) {
        btn.classList.add("pointer_events_none");
    });
    selected_tab_idx = 0;
}
function contentListClicked(event) {
    event.preventDefault();
    if (is_loading)
        return;
    let clickCuid = this.dataset.cuid;
    is_loading = true;
    is_load_content = true;
    changeContent(clickCuid);
    selectContent();
}
function changeContent(cuid) {
    is_change_cuid = (!is_first_load && selected_cuid != cuid);
    selected_cuid = cuid;
    selected_content_type = getContentType(cuid);
    deleteNotification();
    changeMenu('0');
}
function selectContent() {
    let currentSelectObj = document.querySelectorAll(".GameItem." + CONTENT_SELECTED_CLASS);
    for (let i = 0; i < currentSelectObj.length; i++) {
        currentSelectObj[i].classList.remove(CONTENT_SELECTED_CLASS);
    }
    let selectObj = document.querySelector(".GameItem[data-cuid='" + selected_cuid + "']");
    selectObj.classList.add(CONTENT_SELECTED_CLASS);
    let gameTitle = document.querySelector(".Notification__gameTitle>span");
    let contentName = "";
    if (selected_content_type === CONTENT_TYPE_PC) {
        contentName = contentlist_pc[selected_cuid].name;
    }
    else if (selected_content_type === CONTENT_TYPE_BROWSER) {
        contentName = contentlist_browser[selected_cuid].name;
    }
    gameTitle.innerText = contentName;
    if (!is_first_load) {
        clearContent();
    }
    ipcRenderer.send("get_content_tabs", selected_cuid);
}
function updateContentList(parentObj, list, contentType) {
    if (contentType === CONTENT_TYPE_BROWSER) {
        document.querySelector(".GameSelector.-browser").classList.toggle("c_hide", Object.keys(list).length == 0);
    }
    let cloneObj = parentObj.cloneNode(false);
    Object.keys(list).forEach(function (key) {
        let content = list[key];
        if (!selected_cuid) {
            selected_cuid = content.cuid;
        }
        let game_item = createEle("a", "GameItem");
        if (selected_cuid === content.cuid) {
            game_item.classList.add(CONTENT_SELECTED_CLASS);
            selected_content_type = contentType;
        }
        game_item.dataset.cuid = content.cuid;
        game_item.dataset.contenttype = contentType.toString();
        game_item.addEventListener("click", contentListClicked, false);
        let thumbnail = createEle("div", "GameItem__thumbnail");
        let img = document.createElement('img');
        img.src = content.banner_url;
        thumbnail.appendChild(img);
        game_item.appendChild(thumbnail);
        let info = createEle("div", "GameItem__info");
        let info_notice = createEle("div", "GameItem__notice");
        info_notice.innerText = "● NEW";
        if (content.is_new) {
            info_notice.classList.add("-show");
        }
        info.appendChild(info_notice);
        let info_title = createEle("div", "GameItem__title");
        let content_name = replaceBr(content.list_display_name);
        info_title.innerHTML = content_name;
        info.appendChild(info_title);
        let badge = createEle("span", "badge c_hide");
        badge.innerText = "";
        badge.dataset.badgecategory = UPDATE_BADGE_LIST.indexOf("content").toString(10);
        badge.dataset.selectid = content.cuid;
        game_item.appendChild(badge);
        game_item.appendChild(info);
        cloneObj.appendChild(game_item);
    });
    parentObj.parentNode.replaceChild(cloneObj, parentObj);
}
function showContentTerms(confirm_btn) {
    ipcRenderer.send("show_content_terms", "cuid=" + selected_cuid + "&confirm_btn=" + confirm_btn);
}
function showContentInstall() {
    ipcRenderer.send("open_install_window", selected_cuid);
}
function decideSelectedCuid() {
    if (selected_cuid && !(selected_cuid in contentlist_pc) && !(selected_cuid in contentlist_browser)) {
        console.log("[decideSelectedCuid] selected_content none.");
        selected_cuid = null;
        selected_tab_type = 0;
    }
}
function getTargetTabTypeIndex(target_tab_type) {
    let result_index = -1;
    if (target_tab_type != null || target_tab_type !== -1) {
        for (let i = 0; i < tablist.length; i++) {
            if (tablist[i].tab_type === target_tab_type) {
                result_index = i;
                break;
            }
        }
    }
    return result_index;
}
function contentTabClicked(event) {
    event.preventDefault();
    if (is_loading)
        return;
    let clickTabIndex = parseInt(this.dataset.list_index);
    if (selected_tab_idx === clickTabIndex)
        return;
    is_loading = true;
    coverWrapped = true;
    is_load_content = false;
    selected_tab_idx = clickTabIndex;
    serectTab();
}
function serectTab() {
    let loadUrl = null;
    if (priority_tab_type != -1) {
        let tab_index = getTargetTabTypeIndex(priority_tab_type);
        if (tab_index != -1) {
            selected_tab_idx = tab_index;
        }
        console.log("tabindex:" + tab_index + " selected_tab_idx:" + selected_tab_idx + " priority_tab_type:" + priority_tab_type);
        if (priority_url) {
            loadUrl = priority_url;
        }
        priority_tab_type = -1;
        priority_url = null;
    }
    let tab = tablist[selected_tab_idx];
    selected_tab_type = tab.tab_type;
    console.log('tab:' + tab.title + " select:" + selected_tab_idx);
    if (!loadUrl) {
        loadUrl = tab.url;
    }
    if (load_achievement_url) {
        loadUrl = load_achievement_url;
        load_achievement_url = null;
    }
    if (is_first_load) {
        setLoadIframeTimeout('.Web', loadUrl);
        changeFontColorSet(selected_content_data['font_color']);
        if (startAnimeSkip) {
            is_load_content = true;
            var container = document.querySelector(".Container");
            container.classList.remove('-unvisible');
            changeBackGround('rgba(255,255,255,0)', selected_content_data['background_color'], 0);
            changeBackgroundMainMenu(selected_content_data['background_color'], 0);
            changeMenu('1', 'show');
            changeFooter('show');
            var mainbg = document.querySelector('.GameCover__container--bottom');
            mainbg.style.background = selected_content_data['background_color'];
            changeIframe('.Web', '0');
            setTimeout(function () {
                is_first_load = false;
                changeCoverOut();
                TweenMax.to('.Preload', .8, { opacity: 0, zIndex: 0, ease: "Power2.easeOut" });
            }, 700);
            ipcRenderer.send('load_anime_completed', '');
        }
        else {
            console.log("start Preload");
            setTimeout(function () {
                changeCoverOut();
                changeMenu('1', 'show');
                changeFooter('show');
                setTimeout(function () {
                    removePreload();
                    is_first_load = false;
                    ipcRenderer.send('load_anime_completed', '');
                }, 200);
            }, 2000);
            preCallProgressBar();
            setTimeout(function () {
                var container = document.querySelector(".Container");
                container.classList.remove('-unvisible');
            }, 300);
            console.log("end Preload");
            setTimeout(function () {
                changeBackGround('rgba(255,255,255,0)', selected_content_data['background_color'], 100);
                changeBackgroundMainMenu(selected_content_data['background_color'], 200);
                var mainbg = document.querySelector('.GameCover__container--bottom');
                mainbg.classList.add('active');
                mainbg.style.background = selected_content_data['background_color'];
                setTimeout(function () {
                    changeIframe('.Web', '0');
                }, 900);
            }, 600);
        }
    }
    else {
        if (is_load_content) {
            setTimeout(function () {
                changeFontColorSet(selected_content_data['font_color']);
                setLoadIframeTimeout('.Web', loadUrl);
                changeIframe('.Web', '0');
            }, 700);
            changeCover(selected_content_data);
            const progressInfo = document.querySelector(".ProgressInfo");
            let progressDisplay = window.getComputedStyle(progressInfo, '').display;
            if (is_change_cuid && progressDisplay != "none") {
                TweenMax.to(".Web", 0, {
                    height: "calc(100vh - 145px)"
                });
                TweenMax.to('.ProgressInfo', 0, {
                    y: 40,
                    ease: "Power4.easeOut",
                    onComplete: function () {
                        progressInfo.style.display = "none";
                    }
                });
            }
            is_change_cuid = false;
        }
        else {
            showCartainLink(loadUrl);
        }
    }
}
var before_iframe_timeout_timer = null;
var iframe_timeout_timer = null;
var timeOut = null;
function setLoadIframeTimeout(selector, load_url, diplayed = false) {
    if (iframe_timeout_timer) {
        clearTimeout(iframe_timeout_timer);
    }
    if (diplayed) {
        loadIframeDisplayed(selector, load_url);
    }
    else {
        loadIframe(selector, load_url);
    }
    iframe_timeout_timer = setTimeout(function () {
        if (diplayed) {
            loadIframeDisplayed(selector, IFRAME_ERROR_URL);
        }
        else {
            loadIframe(selector, IFRAME_ERROR_URL);
        }
    }, IFRAME_TIMEOUT);
}
function loadIframeDisplayed(className, dataHref) {
    const iframe = document.querySelector(className);
    iframe.src = dataHref;
}
function sendGameTab() {
    let gameTab = document.querySelector(`.MainMenu .MainMenu__button[data-list_index='${TAB_TYPE_GAME_ID}']`);
    if (gameTab) {
        ShowSetting(false, 'option');
        setTimeout(function () {
            gameTab.click();
        }, 800);
    }
}
function updateContentTab() {
    let parentObj = document.querySelector('.MainMenu');
    let cloneObj = parentObj.cloneNode(false);
    setInstallPermit();
    for (let i = 0; i < tablist.length; i++) {
        let tab = createEle("a", "MainMenu__button");
        tab.dataset.list_index = i.toString(10);
        tab.addEventListener("click", contentTabClicked, false);
        let item = createEle("div", "MainMenu__item");
        let icon = createEle("div", "MainMenu__icon");
        let icon_class = TAB_CLASS_LIST[tablist[i].tab_type];
        icon.classList.add(icon_class);
        let img = document.createElement('img');
        img.src = "/img/index/MainButton__icon-" + icon_class + ".svg";
        icon.appendChild(img);
        if (TAB_TYPE_NEWS_ID == tablist[i].tab_type) {
            let badge = createEle("div", "MainMenu__noticeBadge c_hide");
            let badgeCir = createEle("div", "MainMenu__noticeBadgeCircle");
            let badgeVal = createEle("div", "MainMenu__noticeBadgeValue");
            badgeVal.innerText = "!";
            badgeCir.appendChild(badgeVal);
            badge.appendChild(badgeCir);
            let badge_cate = UPDATE_BADGE_LIST.indexOf("news");
            badge.dataset.badgecategory = badge_cate.toString(10);
            icon.appendChild(badge);
        }
        item.appendChild(icon);
        let tabname = createEle("div", "MainMenu__itemTitle");
        tabname.innerText = tablist[i].title;
        item.appendChild(tabname);
        tab.appendChild(item);
        cloneObj.appendChild(tab);
    }
    cloneObj.classList.remove("c_hide");
    parentObj.parentNode.replaceChild(cloneObj, parentObj);
    serectTab();
    updateState();
}
function setInstallPermit() {
    const btn = document.getElementById("btn_content_install");
    const enabled = selected_content_data['install_permit'];
    if (selected_content_type == CONTENT_TYPE_PC) {
        contentlist_pc[selected_cuid].install_permit = enabled;
    }
    else {
        contentlist_browser[selected_cuid].install_permit = enabled;
    }
    btn.classList.toggle("disabled_install", !enabled);
}
function getNotificationVersion(clear = false) {
    try {
        if (!selected_content_data)
            return;
        let versionUrl = selected_content_data.news_version_url;
        let pageUrl = selected_content_data.news_detail_url;
        if (clear) {
            deleteNotification();
        }
        if (!versionUrl || !pageUrl)
            return;
        var result = XMLHttp.get(versionUrl, {}, 'json');
        let rootUrl = versionUrl.substring(0, versionUrl.lastIndexOf('/') + 1);
        console.log(`versionUrl=${versionUrl} pageUrl=${pageUrl} rootUrl=${rootUrl}`);
        result.then((response) => {
            if (response.version) {
                getNotificationList(rootUrl, response.version, pageUrl);
            }
        }).catch((error) => {
            console.log('[getNotificationVersion CHATCH ERROR]' + error);
        });
    }
    catch (e) {
        console.log('[getNotificationVersion ERROR]' + e.message);
    }
}
function getNotificationList(rootUrl, version, pageUrl) {
    try {
        var result = XMLHttp.get(`${rootUrl}${version}.json`, {}, 'json');
        result.then((response) => {
            if (response) {
                if ('on-maintenance' in response) {
                    console.log(`[Notification responce] maintenance:${response['on-maintenance']} status:${response['maintenance-status']} message:${response['maintenance-message']}`);
                    setContentMaintenanceNotice(response['on-maintenance'], response['maintenance-status'], response['maintenance-message']);
                    if (!response['on-maintenance']) {
                        replaceNotification(response.news, pageUrl);
                    }
                }
                else {
                    setContentMaintenanceNotice(false, '', '');
                    replaceNotification(response, pageUrl);
                }
            }
        }).catch((error) => {
            console.log('[getNotificationList CHATCH ERROR]' + error);
        });
    }
    catch (e) {
        console.log('[getNotificationList ERROR]' + e.message);
    }
}
function replaceNotification(notifications, pageUrl) {
    let parentObj = document.querySelector(`.Notification__list`);
    let cloneObj = parentObj.cloneNode(false);
    notifications.forEach((item) => {
        let locations = item.locations;
        let isMessages = (locations.filter(loc => loc.location_slug === 'top_launcher').length > 0);
        if (isMessages) {
            let newItem = createNotification(item.id, item.news_title, pageUrl, item.news_url);
            cloneObj.appendChild(newItem);
        }
    });
    parentObj.parentNode.replaceChild(cloneObj, parentObj);
    scrollNews();
}
function createNotification(newsId, title, pageUrl, newsUrl) {
    let div = createEle('div', 'Notification__message');
    let span = createEle('span', 'Notification__newIcon');
    let img = createEle('img', 'Notification__newIconImg');
    img.src = '/img/index/Notification__maintenanceIcon--info.png';
    div.appendChild(span).appendChild(img);
    let text = document.createTextNode(title);
    let a = createEle('a', 'Notification__messageLink');
    if (newsUrl) {
        console.log('[new_url]' + newsUrl);
        a.href = newsUrl;
        a.addEventListener('click', function (e) {
            e.preventDefault();
            ipcRenderer.send("browser_open", this.href);
        }, false);
    }
    else {
        a.href = addUrlParam(pageUrl, [{ key: 'id', value: newsId }]);
        a.addEventListener('click', function (e) {
            e.preventDefault();
            sendContent(selected_cuid, TAB_TYPE_NEWS_ID, this.href);
        }, false);
    }
    div.appendChild(a).appendChild(text);
    return div;
}
function setContentMaintenanceNotice(isMaintenance, maintenanceStatus, message) {
    let notice = document.querySelector('.Notification');
    let maintenanceNotice = document.querySelector('.Notification__maintenanceMessage');
    maintenanceNotice.innerText = message;
    notice.classList.remove('-notification', '-warning', '-alert');
    if (isMaintenance) {
        notice.classList.add(`-${maintenanceStatus}`);
    }
    notice.classList.toggle('-maintenance', isMaintenance);
}
function deleteNotification() {
    let notificationObj = document.querySelector(`.Notification__list`);
    var cloneObj = notificationObj.cloneNode(false);
    notificationObj.parentNode.replaceChild(cloneObj, notificationObj);
    setContentMaintenanceNotice(false, '', '');
}
function updateActiveUser(num) {
    let activeUserObj = document.querySelector('.Operation__chatIconTextNumber');
    activeUserObj.innerText = num;
}
var watch_update_state_timer = null;
var contentStateIntervalMs = 2000;
var inProgressState = false;
function setWatchUpdateState() {
    stopWatchUpdateState();
    if (selected_content_type === CONTENT_TYPE_PC) {
        console.log('[setWatchUpdateState] start => ' + selected_cuid);
        reqContentProgressState();
        watch_update_state_timer = window.setInterval(reqContentProgressState, contentStateIntervalMs);
    }
    else {
        dispSendBrowser();
    }
}
function updateState() {
    stopWatchUpdateState();
    if (selected_content_type === CONTENT_TYPE_PC) {
        console.log('[updateState] ' + selected_cuid);
        reqContentState();
    }
    else {
        dispSendBrowser();
    }
}
function changeWatchUpdateStateInterval(inProgress) {
    try {
        if (selected_content_type === CONTENT_TYPE_PC) {
            if (inProgressState !== inProgress) {
                if (watch_update_state_timer != null) {
                    window.clearInterval(watch_update_state_timer);
                }
                inProgressState = inProgress;
                if (inProgress) {
                    watch_update_state_timer = window.setInterval(reqContentProgressState, contentStateIntervalMs);
                }
            }
        }
    }
    catch (e) {
        console.log('[changeWatchUpdateStateInterval Error] ' + e.message);
    }
}
function stopWatchUpdateState() {
    if (watch_update_state_timer != null) {
        window.clearInterval(watch_update_state_timer);
    }
    waiting_action_complete = null;
    before_content_state = null;
    if (selected_content_type === CONTENT_TYPE_PC) {
        if (waiting_installing.indexOf(selected_cuid) !== -1) {
            if (!selected_content_options || !selected_content_options.installed) {
                waiting_action_complete = CONTENT_ACTION_INSTALL;
            }
            else {
                waiting_installing = waiting_installing.filter(n => n !== selected_cuid);
            }
        }
        if (waiting_uninstalling.indexOf(selected_cuid) !== -1) {
            if (selected_content_options && selected_content_options.installed) {
                waiting_action_complete = CONTENT_ACTION_UNINSTALL;
            }
            else {
                waiting_uninstalling = waiting_uninstalling.filter(n => n !== selected_cuid);
            }
        }
    }
}
function reqContentState() {
    if (selected_cuid && selected_content_type === CONTENT_TYPE_PC) {
        ipcRenderer.send("bs_get_content_state", selected_cuid);
    }
}
function reqContentProgressState() {
    if (selected_cuid && selected_content_type === CONTENT_TYPE_PC) {
        ipcRenderer.send("bs_get_content_state_progress", selected_cuid);
    }
}
function updateContentState(result) {
    try {
        const state = ACTION_CHANNEL_LIST[result.content_state];
        is_content_maintenance = (state === "Maintenance");
        if (before_content_state !== null) {
            if ((before_content_state === "Maintenance" && state !== "Maintenance")
                || (before_content_state !== "Maintenance" && state === "Maintenance")) {
                getNotificationVersion(true);
            }
        }
        before_content_state = state;
        const progressTitle = document.querySelector(".ProgressTime > .ProgressTime__title");
        const progressTime = document.querySelector(".ProgressTime > .ProgressTime__time");
        const progressInfo = document.querySelector(".ProgressInfo");
        const progress = document.querySelector('.ProgressBar__bar--present');
        const progressVolume = document.querySelector(".ProgressVolume");
        const progressTimeParent = document.querySelector(".ProgressTime");
        let btn_text = "";
        let msg = "";
        let action = "";
        let progress_disp = false;
        let progress_count = false;
        let optionEnabled = false;
        if (state === "NotInstalled") {
            btn_text = BUTTON_TEXT_INSTALL;
            action = "content_install";
            let panelObj = document.querySelector('.OptionPanel');
            let notInstalled = panelObj.classList.contains('not_installed');
            optionEnabled = (notInstalled || has_support_url);
        }
        else if (state === "Standby") {
            btn_text = BUTTON_TEXT_PLAY;
            action = "bs_content_play";
            optionEnabled = true;
        }
        else if (state === "UpdateRequired") {
            btn_text = BUTTON_TEXT_UPDATE;
            action = "bs_content_update";
            optionEnabled = true;
        }
        else if (state === "InstallDownloading" || state === "Installing") {
            btn_text = BUTTON_TEXT_INTERRUPTION;
            action = "send_content_change_pause_install";
            progress_disp = true;
        }
        else if (state === "UpdateDownloading" || state === "Updating") {
            btn_text = BUTTON_TEXT_INTERRUPTION;
            action = "send_content_change_pause_update";
            progress_disp = true;
        }
        else if (state === "Restriction") {
            msg = result.caption;
        }
        else if (state === "Playing") {
        }
        else if (state === "Relocating") {
            btn_text = (result.is_newest ? BUTTON_TEXT_PLAY : BUTTON_TEXT_UPDATE);
            action = "send_content_change_pause_relocate";
            progress_disp = true;
            progress_count = true;
        }
        else if (state === "Checking") {
            btn_text = BUTTON_TEXT_PAUSE;
            action = "send_content_change_pause_check";
            progress_disp = true;
            progress_count = true;
        }
        else if (state === "PreparingUnInstall") {
            btn_text = (result.is_newest ? BUTTON_TEXT_PLAY : BUTTON_TEXT_UPDATE);
            action = "send_content_change_pause_preparing_uninstall";
            progress_disp = true;
            progress_count = true;
            selected_content_options = null;
        }
        else if (state === "UnInstalling") {
            btn_text = (result.is_newest ? BUTTON_TEXT_PLAY : BUTTON_TEXT_UPDATE);
            action = "send_content_change_pause_uninstalling";
            progress_disp = true;
            progress_count = true;
            selected_content_options = null;
        }
        else if (state === "Maintenance") {
            btn_text = BUTTON_TEXT_MAINTENANCE;
            action = "bs_content_play";
            optionEnabled = true;
        }
        else {
            progress_disp = true;
        }
        changeWatchUpdateStateInterval(progress_disp);
        if (waiting_action_complete !== null) {
            if (waiting_action_complete === CONTENT_ACTION_INSTALL) {
                if (installCompleteState.indexOf(state) != -1) {
                    console.log('[install complete]');
                    waiting_action_complete = null;
                    waiting_installing = waiting_installing.filter(n => n !== selected_cuid);
                    setTimeout(function () {
                        reloadSelectContent();
                    }, 500);
                }
            }
            else if (waiting_action_complete === CONTENT_ACTION_UNINSTALL) {
                if (uninstallCompleteState.indexOf(state) != -1) {
                    console.log('[uninstall complete]');
                    waiting_action_complete = null;
                    waiting_uninstalling = waiting_uninstalling.filter(n => n !== selected_cuid);
                    setTimeout(function () {
                        reloadSelectContent();
                    }, 1500);
                }
            }
        }
        if (selected_content_type === CONTENT_TYPE_BROWSER || !selected_content_options) {
            if (state !== "NotInstalled" || !has_support_url) {
                optionEnabled = false;
            }
        }
        if (state === "Standby" && !selected_content_options) {
            ipcRenderer.send("get_content_options", selected_cuid);
        }
        let calculating = false;
        let present = result.installed_size / result.total_size * 100.0;
        if (present > 100) {
            present = 100;
        }
        var beforProgressWidth = '0%';
        var afterProgressWidth = present + '%';
        if (result.rest_minute < 0 || result.installed_size <= 0 && result.total_size <= 0) {
            calculating = true;
            progress.style.width = '0%';
        }
        else {
            beforProgressWidth = (progress.style.width ? progress.style.width : '0%');
            let beforProgressWidthNum = parseInt(beforProgressWidth);
            if (beforProgressWidthNum > present) {
                beforProgressWidth = '0%';
            }
        }
        let progressDisplay = window.getComputedStyle(progressInfo, '').display;
        if (progress_disp) {
            let volume = "";
            if (!calculating) {
                if (progress_count) {
                    volume = result.installed_size + " / " + result.total_size;
                }
                else {
                    volume = convertSizeStringMb(result.installed_size) + " / " + convertSizeStringMb(result.total_size);
                }
            }
            progressVolume.innerText = volume;
            progressTitle.classList.toggle('c_hide', result.is_waiting || result.is_download_completed);
            if (result.is_waiting) {
                progressTime.innerText = result.caption;
            }
            else if (result.is_download_completed) {
                progressTime.innerText = PROGGRESS_DOWNLOAD_COMPLETED;
            }
            else {
                progressTime.innerText = (calculating ? PROGGRESS_CALCULATING : (result.rest_minute.toString() + PROGGRESS_TIME_UNIT));
            }
            progressTimeParent.classList.toggle("Calculating", result.is_calculating);
            if (result.is_calculating) {
                progressTime.innerText = PROGGRESS_CALCULATING;
            }
            if (progressDisplay == 'none') {
                progressInfo.style.display = 'block';
                TweenMax.to(".Web", .3, {
                    delay: .7,
                    height: "calc(100vh - 185px)",
                    ease: "Power4.easeOut"
                });
                TweenMax.fromTo(progress, 1.5, { width: beforProgressWidth }, {
                    width: present + "%",
                    ease: "Power4.easeInOut"
                });
                TweenMax.fromTo('.ProgressInfo', .3, {
                    y: 40
                }, {
                    delay: .7,
                    y: 0,
                    ease: "Power4.easeOut",
                    onComplete: function () {
                        if (!calculating) {
                            TweenMax.fromTo(progress, 1.5, {
                                width: beforProgressWidth
                            }, {
                                width: afterProgressWidth,
                                ease: "Power4.easeInOut"
                            });
                        }
                    }
                });
            }
            else {
                if (!calculating) {
                    TweenMax.fromTo(progress, 1.5, {
                        width: beforProgressWidth
                    }, {
                        width: afterProgressWidth,
                        ease: "Power4.easeInOut"
                    });
                }
            }
        }
        else {
            if (progressDisplay != 'none') {
                TweenMax.to(".Web", 2, {
                    delay: "1.0",
                    height: "calc(100vh - 145px)"
                });
                TweenMax.to('.ProgressInfo', .3, {
                    y: 40,
                    ease: "Power4.easeOut",
                    onComplete: function () {
                        progressVolume.innerText = "";
                        progressInfo.style.display = "none";
                    }
                });
            }
        }
        setInstallBtn(state, action, btn_text, result.is_canceling);
        optionsEnabled(optionEnabled);
        if (state === "NotInstalled" && optionEnabled) {
            setInstalledOptions(false);
        }
    }
    catch (e) {
        console.log('[updateContentState Error] ' + e.message);
    }
}
function reloadSelectContent() {
    is_loading = true;
    is_load_content = true;
    changeContent(selected_cuid);
    selectContent();
}
function setInstallBtn(state, action = "", btn_text = "", canceling = false) {
    const btn = document.getElementById("btn_content_install");
    const icon = document.querySelector('.Action__install .Action__icon img');
    const enabled = selected_content_data['install_permit'];
    const otherLoginBtn = document.getElementById("other_user_login");
    btn.dataset.action = action;
    let allowLoginType = (selected_content_data.login_type == LOGIN_TYPE_V2 || selected_content_data.login_type == LOGIN_TYPE_V3);
    if (otherLoginBtn) {
        otherLoginBtn.classList.toggle('c_hide', !(allowLoginType && btn.dataset.action === 'bs_content_play'));
    }
    if (state === "Standby") {
        icon.src = "/img/index/ActionButton__icon--start.svg";
    }
    else {
        icon.src = "/img/index/ActionButton__icon--install.svg";
    }
    icon.classList.toggle("c_hide", !btn_text);
    btn.querySelector(".Action__title--install").innerText = btn_text;
    btn.classList.toggle("pointer_events_none", (btn_text === "" || state == "UnInstalling" || state == "PreparingUnInstall" || state == "Relocating" || canceling));
    btn.classList.toggle("disabled_install", !enabled);
}
function dispSendBrowser() {
    const btn = document.getElementById("btn_content_install");
    const progressInfo = document.querySelector(".ProgressInfo");
    const icon = document.querySelector('.Action__install .Action__icon img');
    const progressVolume = document.querySelector(".ProgressVolume");
    const optionBtn = document.querySelector(".Action__option");
    icon.src = "/img/index/ActionButton__icon--start.svg";
    btn.dataset.action = "browser_title_play";
    progressVolume.innerText = "";
    TweenMax.to(".Web", 2, {
        delay: "1.0",
        height: "calc(100vh - 145px)"
    });
    TweenMax.to(progressInfo, .3, {
        y: 40,
        ease: "Power4.easeOut",
        onComplete: function () {
            progressInfo.style.display = "none";
        }
    });
    btn.querySelector(".Action__title--install").innerText = BUTTON_TEXT_PLAY;
    btn.classList.toggle("pointer_events_none", false);
    optionBtn.classList.toggle("pointer_events_none", true);
}
function updateCommunityState(state) {
    let infoState = document.querySelector(".UserInfo__headUserStatus");
    infoState.innerText = COMMUNITY_STATE_DISP_LIST[state];
}
function updateProfile(accountdata) {
    setAccountUniqueConfig();
    updateCommunityState(accountdata.communityState);
    const infoUserName = document.querySelector(".UserInfo__headUserName");
    const settingUserName = document.getElementById("setting_account_name");
    infoUserName.innerText = accountdata.account.name;
    if (settingUserName) {
        settingUserName.value = accountdata.account.name;
    }
    const infoUserComment = document.querySelector(".UserInfo__headUserComment");
    const settingUserComment = document.getElementById("setting_account_comment");
    infoUserComment.innerText = accountdata.account.comment;
    if (settingUserComment) {
        settingUserComment.value = accountdata.account.comment;
    }
    const friendCode = document.querySelector(".UserInfo__friendCodeNumber");
    friendCode.innerText = accountdata.account.friend_code;
    updateAchievement(accountdata.achievementInfo);
    const icon = document.querySelector(".UserInfo__headUserIcon>img");
    icon.src = accountdata.account.source_url;
    replaceGameList(accountdata.account_content_list);
    selectedIcon();
}
function updateAchievement(achievementInfo) {
    const level__level = document.querySelector(".Operation__level--level");
    level__level.innerHTML = achievementInfo.level;
    const coinQuantity = document.querySelector(".Operation__coinQuantity");
    coinQuantity.innerHTML = achievementInfo.coin;
    const CoinModal__coinQuantity = document.querySelector('.CoinModal__coinQuantity');
    CoinModal__coinQuantity.innerHTML = achievementInfo.coin;
    const Achievement__level = document.querySelector(".Achievement__level");
    Achievement__level.innerHTML = achievementInfo.level;
    const Achievement__levelBarContainer = document.querySelector(".Achievement__levelBarContainer p");
    Achievement__levelBarContainer.innerHTML = achievementInfo.point + "%";
    const Achievement__levelBar = document.querySelector(".Achievement__levelBar div");
    Achievement__levelBar.style.width = achievementInfo.point + "%";
    const Achievement__trophyContainer = document.querySelectorAll(".Achievement__trophyContainer span");
    Achievement__trophyContainer[0].innerHTML = achievementInfo.platinum;
    Achievement__trophyContainer[1].innerHTML = achievementInfo.gold;
    Achievement__trophyContainer[2].innerHTML = achievementInfo.silver;
    Achievement__trophyContainer[3].innerHTML = achievementInfo.bronze;
    let Achievement__contents = document.querySelector("#Achievement__contents");
    let cloneObj = Achievement__contents.cloneNode(false);
    for (let i = 0; i < achievementInfo.contents.length; i++) {
        let item = achievementInfo.contents[i];
        let titleContainer = createEle("a", "Achievement__titleContainer");
        titleContainer.dataset.href = HOST_URL + "/Achievement/ContentDetail?content_unique_id=" + item.content_unique_id;
        titleContainer.target = "Web";
        titleContainer.dataset.cuid = item.content_unique_id;
        titleContainer.addEventListener("click", function (event) {
            load_achievement_url = this.dataset.href;
            changeContent(item.content_unique_id);
            selectContent();
            changeCover(selected_content_data);
            var Achievement = document.querySelector('.Achievement');
            TweenMax.to(Achievement, .4, {
                opacity: "0",
                onComplete: function () {
                    Achievement.style.display = "none";
                }
            });
        });
        let titleImage = createEle("div", "Achievement__titleImage");
        let img = createEle("img");
        img.src = item.icon_url;
        titleImage.appendChild(img);
        titleContainer.appendChild(titleImage);
        let titleGaugeContainer = createEle("div", "Achievement__titleGaugeContainer");
        let title = createEle("div", "Achievement__title");
        title.innerHTML = item.name;
        let titleBarContainer = createEle("div", "Achievement__titleBarContainer");
        let titleBar = createEle("div", "Achievement__titleBar");
        let div = createEle("div");
        let p = createEle("p");
        div.style.width = p.innerHTML = item.point + "%";
        titleBar.appendChild(div);
        titleBarContainer.appendChild(titleBar);
        titleBarContainer.appendChild(p);
        title.appendChild(titleBarContainer);
        titleGaugeContainer.appendChild(title);
        titleContainer.appendChild(titleGaugeContainer);
        let titleTrophyContainer = createEle("div", "Achievement__titleTrophyContainer");
        for (let j = 0; j < 4; j++) {
            let img = createEle("img");
            let span = createEle("span");
            switch (j) {
                case 0:
                    img.src = "/img/level/Level__trophy--platinum.png";
                    span.innerHTML = item.platinum;
                    break;
                case 1:
                    img.src = "/img/level/Level__trophy--gold.png";
                    span.innerHTML = item.gold;
                    break;
                case 2:
                    img.src = "/img/level/Level__trophy--silver.png";
                    span.innerHTML = item.silver;
                    break;
                case 3:
                    img.src = "/img/level/Level__trophy--bronze.png";
                    span.innerHTML = item.bronze;
                    break;
            }
            let titleTrophy = createEle("div", "Achievement__titleTrophy");
            titleTrophy.appendChild(img);
            titleTrophy.appendChild(span);
            titleTrophyContainer.appendChild(titleTrophy);
        }
        titleContainer.appendChild(titleTrophyContainer);
        cloneObj.appendChild(titleContainer);
    }
    Achievement__contents.parentNode.replaceChild(cloneObj, Achievement__contents);
}
function updateAchievementContent(achievementContentInfo) {
    let childFrame = document.querySelector("#child-frame");
    childFrame.contentWindow.updateAchievementContent(achievementContentInfo);
}
function updateShop(model) {
    let localize = model.localize;
    let shopInfo = model.info;
    let CoinModal__productContainer = document.querySelector(".CoinModal__productContainer");
    let cloneObj = CoinModal__productContainer.cloneNode(false);
    for (let i = 0; i < shopInfo.items.length; i++) {
        let item = shopInfo.items[i];
        let acquire = item.isOwn ? "acquire" : "unacquired";
        let CoinModal__product = createEle("div", "CoinModal__product");
        CoinModal__product.classList.add(acquire);
        CoinModal__product.id = "shopid_" + item.item_id;
        CoinModal__product.dataset.desc = item.description;
        CoinModal__product.dataset.use = item.isUse;
        CoinModal__product.dataset.shopid = "shopid_" + item.item_id;
        let CoinModal__productImage = createEle("div", "CoinModal__productImage");
        let productImage = createEle("img");
        productImage.src = item.icon_url;
        CoinModal__productImage.appendChild(productImage);
        let CoinModal__productGameTitle = createEle("div", "CoinModal__productGameTitle");
        CoinModal__productGameTitle.innerHTML = item.title;
        let CoinModal__productTitleContainer = createEle("div", "CoinModal__productTitleContainer");
        let CoinModal__productTitle = createEle("div", "CoinModal__productTitle");
        CoinModal__productTitle.innerHTML = item.name;
        let CoinModal__productCoin = createEle("div", "CoinModal__productCoin");
        let CoinModal__productCoinIcon = createEle("div", "CoinModal__productCoinIcon");
        let productCoinIcon = createEle("img");
        productCoinIcon.src = "/img/shop-coin/CoinModal__coinIcon.png";
        CoinModal__productCoinIcon.appendChild(productCoinIcon);
        let CoinModal__productCoinCost = createEle("div", "CoinModal__productCoinCost");
        CoinModal__productCoinCost.innerHTML = item.coin;
        CoinModal__productCoin.appendChild(CoinModal__productCoinIcon);
        CoinModal__productCoin.appendChild(CoinModal__productCoinCost);
        CoinModal__productTitleContainer.appendChild(CoinModal__productTitle);
        CoinModal__productTitleContainer.appendChild(CoinModal__productCoin);
        CoinModal__product.appendChild(CoinModal__productImage);
        CoinModal__product.appendChild(CoinModal__productGameTitle);
        CoinModal__product.appendChild(CoinModal__productTitleContainer);
        cloneObj.appendChild(CoinModal__product);
    }
    CoinModal__productContainer.parentNode.replaceChild(cloneObj, CoinModal__productContainer);
    document.querySelectorAll('.CoinModal__product').forEach(function (button) {
        button.addEventListener("click", function (e) {
            let target = e.target;
            let parent = target.parentNode.parentNode;
            let shopid = parent.dataset.shopid;
            const CoinModalDetails__exchangeButton = document.querySelector('.CoinModalDetails__exchangeButton');
            CoinModalDetails__exchangeButton.dataset.shopid = shopid;
            const CoinModalDetails__image = document.querySelector('.CoinModalDetails__image img');
            const CoinModal__productImage = document.querySelector('#' + shopid + ' .CoinModal__productImage img');
            CoinModalDetails__image.src = CoinModal__productImage.src;
            const CoinModalDetails__gameTitle = document.querySelector('.CoinModalDetails__gameTitle');
            const CoinModal__productGameTitle = document.querySelector('#' + shopid + ' .CoinModal__productGameTitle');
            CoinModalDetails__gameTitle.innerHTML = CoinModal__productGameTitle.innerHTML;
            const CoinModalDetails__title = document.querySelector('.CoinModalDetails__title');
            const CoinModal__productTitle = document.querySelector('#' + shopid + ' .CoinModal__productTitle');
            CoinModalDetails__title.innerHTML = CoinModal__productTitle.innerHTML;
            const CoinModalDetails__CoinIcon = document.querySelector('.CoinModalDetails__CoinIcon');
            const CoinModalDetails__CoinCost = document.querySelector('.CoinModalDetails__CoinCost');
            const CoinModal__productCoinCost = document.querySelector('#' + shopid + ' .CoinModal__productCoinCost');
            const CoinModalDetails__explanation = document.querySelector('.CoinModalDetails__explanation');
            const CoinModal__product = document.querySelector('#' + shopid);
            CoinModalDetails__explanation.innerHTML = CoinModal__product.dataset.desc;
            const CoinModalDetails__exchangeTitle = document.querySelector('.CoinModalDetails__exchangeTitle');
            const ConfirmModal__deleteButton = document.querySelector('.ConfirmModal__deleteButton');
            const ConfirmModal__body = document.querySelector('.ConfirmModal__body');
            if (CoinModal__product.classList.contains("acquire")) {
                CoinModalDetails__CoinIcon.style.display = "none";
                CoinModalDetails__CoinCost.innerHTML = localize.find((v) => v.Key === "l_coin_detail_coincost").Value;
                if (CoinModal__product.dataset.use == "false") {
                    CoinModalDetails__exchangeTitle.innerHTML = localize.find((v) => v.Key === "l_coin_detail_use").Value;
                    ConfirmModal__deleteButton.innerHTML = localize.find((v) => v.Key === "l_coin_button_use").Value;
                    ConfirmModal__body.innerHTML = localize.find((v) => v.Key === "l_coin_confirm_use").Value;
                }
                else {
                    CoinModalDetails__exchangeTitle.innerHTML = localize.find((v) => v.Key === "l_coin_detail_release").Value;
                    ConfirmModal__deleteButton.innerHTML = localize.find((v) => v.Key === "l_coin_button_release").Value;
                    ConfirmModal__body.innerHTML = localize.find((v) => v.Key === "l_coin_confirm_release").Value;
                }
            }
            else {
                CoinModalDetails__CoinIcon.style.display = "block";
                CoinModalDetails__CoinCost.innerHTML = CoinModal__productCoinCost.innerHTML;
                CoinModalDetails__exchangeTitle.innerHTML = localize.find((v) => v.Key === "l_coin_detail_exchange").Value;
                ConfirmModal__deleteButton.innerHTML = localize.find((v) => v.Key === "l_coin_button_exchange").Value;
                ConfirmModal__body.innerHTML = localize.find((v) => v.Key === "l_coin_confirm_exchange").Value;
            }
            var CoinModal = document.querySelector('.CoinModal');
            var CoinModal__container = document.querySelector('.CoinModal__container');
            var CoinModal__titleList = document.querySelector('.CoinModal__titleList');
            var SCoinModal__productScrollBarInner = document.querySelector('.SCoinModal__productScrollBarInner');
            var CoinModal__productItems = document.querySelector('.CoinModal__productItems');
            var CoinModalDetails = document.querySelector('.CoinModalDetails');
            TweenMax.to(SCoinModal__productScrollBarInner, .5, {
                opacity: "0",
                onComplete: function () {
                    SCoinModal__productScrollBarInner.style.display = "none";
                }
            });
            TweenMax.to(CoinModal__titleList, .5, {
                opacity: "0",
                onComplete: function () {
                    CoinModal__titleList.style.display = "none";
                }
            });
            TweenMax.to(CoinModal__productItems, .5, {
                opacity: "0",
                onComplete: function () {
                    CoinModal__productItems.style.display = "none";
                    CoinModalDetails.style.opacity = "0";
                    CoinModalDetails.style.display = "flex";
                    TweenMax.to(CoinModalDetails, .5, {
                        opacity: "1"
                    });
                    TweenMax.to(CoinModal__container, .5, {
                        height: 379,
                        padding: "50px 33px 25px",
                        onComplete: function () {
                            TweenMax.to(CoinModal, .5, {
                                height: 531
                            });
                        }
                    });
                }
            });
        });
    });
}
function CoinModalDetailsClose() {
    var CoinModal__backGround = document.querySelector('.CoinModal__backGround');
    var CoinModal = document.querySelector('.CoinModal');
    var CoinModal__container = document.querySelector('.CoinModal__container');
    var CoinModal__titleList = document.querySelector('.CoinModal__titleList');
    var CoinModal__productItems = document.querySelector('.CoinModal__productItems');
    var CoinModalDetails = document.querySelector('.CoinModalDetails');
    var SCoinModal__productScrollBarInner = document.querySelector('.SCoinModal__productScrollBarInner');
    var h = CoinModal__backGround.clientHeight;
    TweenMax.to(CoinModal, .5, {
        height: h - 80,
        onComplete: function () {
            var h = CoinModal.offsetHeight;
            console.log(h);
            TweenMax.to(CoinModal__container, .1, {
                height: h - 54 - 33,
                padding: "18px 10.5px 0"
            });
            SCoinModal__productScrollBarInner.style.display = "block";
            TweenMax.to(SCoinModal__productScrollBarInner, .5, {
                opacity: "1"
            });
            CoinModal__titleList.style.display = "block";
            TweenMax.to(CoinModal__titleList, .5, {
                opacity: "1"
            });
            CoinModal__productItems.style.display = "block";
            CoinModal__container.classList.remove('details');
            TweenMax.to(CoinModal__productItems, .5, {
                opacity: "1",
                onComplete: function () {
                    CoinModalDetails.style.opacity = "1";
                    CoinModalDetails.style.display = "none";
                }
            });
            CoinModal__container.classList.remove('details');
            TweenMax.to(CoinModalDetails, .5, {
                opacity: "0",
                onComplete: function () {
                    CoinModalDetails.style.display = "none";
                    CoinModal__titleList.style.display = "flex";
                    TweenMax.to(CoinModal__titleList, .5, {
                        opacity: "1"
                    });
                    CoinModal__productItems.style.display = "flex";
                    TweenMax.to(CoinModal__productItems, .5, {
                        opacity: "1"
                    });
                }
            });
        }
    });
}
function replaceGameList(list) {
    let parentObj = document.querySelector(".UserInfo__gameList");
    let cloneObj = parentObj.cloneNode(false);
    if (list.length > 0) {
        list.forEach((item) => {
            let boxDiv = createEle("div", "UserInfo__gameListItemBox");
            let imgDiv = createEle("div", "UserInfo__gameListImage");
            let img = createEle("img", "");
            img.src = item.icon_url;
            imgDiv.appendChild(img);
            boxDiv.appendChild(imgDiv);
            let infoDiv = createEle("div", "UserInfo__gameListInfo");
            let head = createEle("div", "UserInfo__gameListInfoHeading");
            head.innerText = item.content_name;
            infoDiv.appendChild(head);
            let sub = createEle("div", "UserInfo__gameListInfoSubHeading");
            sub.innerText = item.user_name;
            infoDiv.appendChild(sub);
            let status = createEle("div", "UserInfo__gameListInfoStatus -publish");
            status.innerHTML = item.disclosure_scope === null ? "&nbsp;" : DISCLOSURE_SCOPE_DISP_LIST[item.disclosure_scope];
            infoDiv.appendChild(status);
            boxDiv.appendChild(infoDiv);
            cloneObj.appendChild(boxDiv);
        });
    }
    parentObj.parentNode.replaceChild(cloneObj, parentObj);
    parentObj = document.querySelector(".UserInfo__gameList");
    parentObj.classList.toggle('c_hide', (list.length === 0));
}
function updateBadge(update_badge_list) {
    for (let i = 0; i < update_badge_list.length; i++) {
        let target_type = update_badge_list[i].badge_type;
        let target = document.querySelectorAll("[data-badgecategory='" + target_type + "']");
        if (target.length) {
            for (let j = 0; j < target.length; j++) {
                let selectid = target[j].dataset.selectid;
                if (selectid) {
                    if (selectid === update_badge_list[i].select_id) {
                        if (update_badge_list[i].value) {
                            target[j].innerText = update_badge_list[i].value;
                            target[j].classList.remove("c_hide");
                        }
                        else {
                            clearBadge(target[j]);
                        }
                    }
                }
                else {
                    if (update_badge_list[i].value) {
                        target[j].innerText = update_badge_list[i].value;
                        target[j].classList.remove("c_hide");
                    }
                    else {
                        clearBadge(target[j]);
                    }
                }
            }
        }
    }
}
function clearBadge(target) {
    if (target) {
        target.classList.add("c_hide");
        target.innerText = "";
    }
}
function updateSocialBadge(disp) {
    let badgeEle = document.querySelector('.Operation__chatIcon .Operation__userIcon--noticeCircle');
    badgeEle.classList.toggle('c_hide', !disp);
}
function setContentOptions(options) {
    selected_content_options = options;
    setSupportPageEnabled(has_support_url);
    if (!options) {
        setInstalledOptions(false);
        if (!has_support_url) {
            optionsEnabled(false);
        }
        return;
    }
    setInstalledOptions(options.installed);
    setCreateShortcutsEnabled(options.is_create_shortcut);
    setSupportPageEnabled(options.has_support_page);
    setContentOption("auto_update", options.auto_update.toString());
    setContentOption("disclosure_scope", options.disclosure_scope.toString(10));
}
function setContentOption(key, selectValue) {
    let eleName = null;
    if (key === "content_language") {
        eleName = "input[name='op_language']";
    }
    else if (key === "auto_update") {
        eleName = "input[name='op_autoupdate']";
    }
    else if (key === "disclosure_scope") {
        eleName = "input[name='ds_scope']";
    }
    if (eleName) {
        let elements = document.querySelectorAll(".OptionPanel " + eleName);
        if (elements) {
            elements.forEach(function (ele) {
                ele.checked = (ele.value === selectValue);
            });
        }
    }
}
function setContentLanguage(list, selectLang) {
    let langs = document.querySelectorAll('.option_content_language .OptionPanel__content');
    langs.forEach(function (item) {
        let radio = item.querySelector('.RadioButton');
        let lang = radio.value;
        item.classList.toggle('c_hide', (list.indexOf(lang) === -1));
    });
}
function setInstalledOptions(installed) {
    let menu = document.querySelector('.OptionPanel');
    menu.classList.toggle('not_installed', !installed);
}
function setCreateShortcutsEnabled(enabled) {
    let ele = document.querySelector('.option_shortcuts_item');
    ele.classList.toggle('c_hide', !enabled);
}
function setSupportPageEnabled(enabled) {
    let ele = document.querySelector('.option_support_page_item');
    ele.classList.toggle('c_hide', !enabled);
}
function createShortcuts() {
    ipcRenderer.send("send_create_shortcuts", selected_cuid);
}
function contentUninstall() {
    ipcRenderer.send("send_content_uninstall", selected_cuid);
}
function contentCheck() {
    ipcRenderer.send("send_content_check", selected_cuid);
}
function debugLoginCode(code) {
    let panel = document.querySelector('#debug_login_code_panel');
    panel.classList.toggle('c_hide', !code);
    let copyBtn = document.querySelector('#debug_login_code_copy');
    let copyInput = document.querySelector('#debug_login_code');
    if (copyBtn && copyInput) {
        copyBtn.innerText = code;
        copyInput.value = code;
        copyBtn.classList.toggle('c_hide', !code);
    }
}
function setOptionsEvent() {
    let optionsObj = document.querySelectorAll(".OptionPanel__itemTitle");
    optionsObj.forEach(function (op) {
        op.addEventListener("click", optionsClicked, false);
    });
    let radioObj = document.querySelectorAll(".OptionPanel__content__inputRadio>input");
    radioObj.forEach(function (op) {
        op.addEventListener("change", optionsRadioClicked, false);
    });
}
function optionsClicked(event) {
    if (selected_content_type !== CONTENT_TYPE_PC)
        return;
    let optiontype = this.dataset.optiontype;
    if (!optiontype)
        return;
    if (optiontype === "uninstall") {
        contentUninstall();
    }
    else if (optiontype === "create_shortcuts") {
        createShortcuts();
    }
    else if (optiontype === "file_repair") {
        contentCheck();
    }
    else if (optiontype === "change_location") {
        ipcRenderer.send("content_select_directory", selected_cuid);
    }
    else if (optiontype === "support_page") {
        ipcRenderer.send("send_support_page", selected_cuid);
    }
    showOptionPanel(false);
}
function optionsRadioClicked(event) {
    if (selected_content_type !== CONTENT_TYPE_PC)
        return;
    let element = document.querySelector(".OptionPanel__content__inputRadio>input[name='" + this.name + "']:checked");
    let value = null;
    if (element) {
        value = element.value;
    }
    if (this.name === "ds_scope") {
        let updatetype = this.dataset.updatetype;
        let sObject = { "update_type": updatetype, "cuid": selected_cuid, "value": value };
        let json = JSON.stringify(sObject);
        console.log(json);
        ipcRenderer.send("change_profile", json);
    }
    else if (this.name.startsWith('unique_op__')) {
        let key = this.dataset.target;
        if (!key)
            return;
        let contentId = getContentId(selected_cuid);
        let uObject = { "update_type": UNIQUE_CONFIG_UPDATE_TYPE, "config_name": key, "content_id": contentId, "value": value, "cuid": selected_cuid };
        let json = JSON.stringify(uObject);
        console.log(json);
        ipcRenderer.send("change_profile", json);
    }
    else {
        let key = this.dataset.target;
        if (!key)
            return;
        let bObject = { "target_name": key, "cuid": selected_cuid, "target_value": value };
        let json = JSON.stringify(bObject);
        console.log(json);
        ipcRenderer.send("bs_change_setting", json);
    }
}
function optionsEnabled(enable) {
    const optionBtn = document.querySelector(".Action__option");
    optionBtn.classList.toggle("pointer_events_none", !enable);
    if (!enable) {
        showOptionPanel(false);
    }
}
function replaceUniqueOption(cuid, unique_configs) {
    deleteUniqueOption();
    if (unique_configs.length > 0) {
        let menuRoot = document.querySelector('.OptionPanel__menu');
        let contentId = getContentId(cuid);
        if (contentId == null)
            return;
        unique_configs.forEach(function (config) {
            let li = createUniqueOption(contentId, config);
            menuRoot.appendChild(li);
        });
    }
}
function deleteUniqueOption() {
    let menu = document.querySelectorAll('.OptionPanel__menu .UniqueOption');
    if (menu.length > 0) {
        menu.forEach(function (li) {
            li.remove();
        });
    }
}
function createUniqueOption(contentId, config) {
    let li = createEle('li', 'OptionPanel__item UniqueOption');
    let titleOp = createEle('a', 'OptionPanel__itemTitle -can-open');
    titleOp.innerText = config.config_text;
    titleOp.addEventListener("click", showOptionRadio, false);
    li.appendChild(titleOp);
    let settingVal = getContentUniqueConfig(contentId, config.config_name);
    let childs = createEle('div', 'OptionPanel__contents');
    let isSettingVal = isSettingValue(settingVal, config.items);
    config.items.forEach(function (item) {
        let isChecked = (isSettingVal ? (settingVal == item.item_name) : item.is_default);
        let content = createUniqueOptionChild(config.config_name, item, isChecked);
        childs.appendChild(content);
    });
    li.appendChild(childs);
    return li;
}
function isSettingValue(settingVal, items) {
    if (settingVal == null)
        return false;
    let result = items.filter(function (x) {
        return x.item_name == settingVal;
    });
    return (result.length > 0);
}
function createUniqueOptionChild(config_name, item, isChecked = false) {
    let content = createEle('label', 'OptionPanel__content');
    let divContent = createEle('div', 'OptionPanel__content__inputRadio');
    let radio = createEle('input', 'RadioButton ignore_rh');
    let radioId = `unique_op__${config_name}__${item.item_name}`;
    radio.type = 'radio';
    radio.name = `unique_op__${config_name}`;
    radio.dataset.target = config_name;
    radio.value = item.item_name;
    radio.checked = isChecked;
    radio.id = radioId;
    radio.addEventListener("change", optionsRadioClicked, false);
    let inputLabel = createEle('label', 'SettingMain__settingItem--inputlabel ignore_rh');
    inputLabel.htmlFor = radioId;
    inputLabel.innerText = item.item_text;
    divContent.append(radio, inputLabel);
    content.appendChild(divContent);
    return content;
}
function showOptionRadio() {
    const toggle_contents = this.nextElementSibling;
    toggle_contents.classList.toggle('-show');
    var elements = document.getElementsByClassName('-can-open');
    Array.prototype.forEach.call(elements, function (element) {
        element.classList.remove('active');
    });
    document.querySelectorAll('.OptionPanel__contents').forEach(function (contents) {
        if (toggle_contents != contents) {
            contents.classList.remove('-show');
        }
    });
}
function getContentId(cuid) {
    let contentId = null;
    if (cuid in contentlist_pc) {
        contentId = contentlist_pc[cuid].content_id;
    }
    else if (cuid in contentlist_browser) {
        contentId = contentlist_browser[cuid].content_id;
    }
    return contentId;
}
function getContentType(cuid) {
    let contentType = null;
    if (cuid in contentlist_pc) {
        contentType = CONTENT_TYPE_PC;
    }
    else if (cuid in contentlist_browser) {
        contentType = CONTENT_TYPE_BROWSER;
    }
    return contentType;
}
function existContent(cuid) {
    if (cuid in contentlist_pc) {
        return true;
    }
    else if (cuid in contentlist_browser) {
        return true;
    }
    return false;
}
function getContentUniqueConfig(contentId, configName) {
    if (profile_data == null || profile_data.unique_config == null)
        return null;
    let configs = profile_data.unique_config.filter(function (item) {
        return item.content_id == contentId && item.name == configName;
    });
    return (configs.length > 0 ? configs[0].value : null);
}
function getContentUniqueConfigs(contentId) {
    if (profile_data == null || profile_data.unique_config == null)
        return null;
    let configs = profile_data.unique_config.filter(function (item) {
        return item.content_id == contentId;
    });
    return configs;
}
function setAccountUniqueConfig() {
    if (!selected_cuid)
        return;
    let contentId = getContentId(selected_cuid);
    if (contentId == null)
        return;
    let contentsConfig = getContentUniqueConfigs(contentId);
    if (contentsConfig == null || contentsConfig.lentth == 0)
        return;
    contentsConfig.forEach(function (item) {
        let radioId = `#unique_op__${item.name}__${item.value}`;
        let target = document.querySelector(`.UniqueOption ${radioId}`);
        if (target) {
            target.checked = true;
        }
    });
}
function changeMaxmizeIconHome(resultAction) {
    changeMaxmizeIcon(resultAction, '/img/index/WindowControl__maximizedButton.png', '/img/index/WindowControl__maximizeButton.png');
}
function clickedContentInstallBtn(action, otherUser) {
    console.log("[btn_content_install click] action : " + action + " otherUser : " + otherUser);
    if (action === "content_install") {
        showContentInstall();
    }
    else if (action === "bs_content_play") {
        var object = { "cuid": selected_cuid, "from_shortcut": false, "other_user_login": otherUser };
        var itemjson = JSON.stringify(object);
        console.log(itemjson);
        ipcRenderer.send(action, itemjson);
    }
    else {
        ipcRenderer.send(action, selected_cuid);
    }
}
$(function () {
    console.log("selected_cuid => " + selected_cuid + " priority_tab_type => " + priority_tab_type + " send_page => " + send_page + " is_first_load =>" + is_first_load);
    if (startAnimeSkip) {
        coverWrapped = true;
        var logo = document.querySelector('.Preload__inner img');
        logo.remove();
        document.querySelector('.GameCover__container').classList.remove('gameContents');
        document.querySelector('.loading').classList.add('active');
        TweenMax.fromTo('.loading', 0, {
            y: -15,
            opacity: 0,
        }, {
            y: 0,
            opacity: 1,
            ease: "Power4.easeIn",
            onComplete: function () {
            }
        });
        TweenMax.to(".Web", 0, {
            delay: "1.0",
            height: "calc(100vh - 145px)"
        });
        TweenMax.fromTo('.GameCover__container--bottom', .7, {
            y: '100%',
            x: '0%'
        }, {
            y: '0%',
            ease: "Power4.easeIn",
            onComplete: function () {
            }
        });
    }
    document.querySelector('.UserInfo__logoutButtonLink').addEventListener("click", function (event) {
        event.preventDefault();
        ipcRenderer.send('user_logout');
    });
    document.querySelector('.Notification__gameTitle').addEventListener('click', sendGameTab, false);
    document.querySelectorAll(".Operation__chatIcon,.Operation__chatIconText").forEach(function (ele) {
        ele.addEventListener("click", function () {
            ipcRenderer.send("show_social_window", "");
        }, false);
    });
    ipcRenderer.on('setting_change_result', function (event, arg) {
        console.log('setting_change_result => ' + arg);
        var result = JSON.parse(arg);
        if (result.target_name === "content_uninstall_start_dialog") {
            let target = document.querySelector(".SettingMain__settingRow[data-updatetype='install_location'][data-cuid='" + result.cuid + "']");
            if (target) {
                let locationChangeBtn = target.querySelector(".DirectorySelector__button");
                if (locationChangeBtn) {
                    locationChangeBtn.classList.toggle('disabled', result.isSuccess);
                }
                let deleteBtn = target.querySelector(".ContentUninstall__button");
                if (deleteBtn) {
                    deleteBtn.textContent = result.isSuccess ? BUTTON_TEXT_CHANGING : BUTTON_TEXT_DELETE;
                    deleteBtn.classList.toggle('disabled', result.isSuccess);
                }
            }
            return;
        }
        if (result.cuid === selected_cuid) {
            setContentOption(result.target_name, result.target_value);
        }
        if (result.target_name === "install_location") {
            let target = document.querySelector(".SettingMain__settingRow[data-updatetype='" + result.target_name + "'][data-cuid='" + result.cuid + "'] input");
            if (target) {
                target.value = result.target_value;
                let btn = target.nextElementSibling;
                btn.textContent = result.isSuccess ? BUTTON_TEXT_CHANGING : BUTTON_TEXT_CHANGE;
                btn.classList.toggle('disabled', result.isSuccess);
                let deleteBtn = document.querySelector(".SettingMain__settingRow[data-updatetype='" + result.target_name + "'][data-cuid='" + result.cuid + "'] .ContentUninstall__button");
                if (deleteBtn) {
                    deleteBtn.classList.toggle('disabled', result.isSuccess);
                }
            }
        }
        else if (result.target_name === "auto_update") {
            let target = document.querySelector(".setting__autoupdate[data-cuid='" + result.cuid + "']");
            if (target) {
                target.checked = result.target_value === "true";
            }
        }
        else if (result.target_name === "avatar_id") {
            let target = document.querySelector('.SettingProfileModal__settingRow');
            if (target) {
                target.classList.toggle("-error", !result.isSuccess);
                if (!result.isSuccess) {
                    let errorArea = target.querySelector(".SettingProfileModal__settingItemErrorMessage");
                    if (!errorArea)
                        return;
                    errorArea.innerText = result.message;
                }
                else {
                    showUserIconSetting(false);
                }
            }
        }
        else if (result.target_name === "comment" || result.target_name === "name") {
            let target = document.querySelector(".SettingMain__settingRow[data-updatetype='" + result.target_name + "']");
            if (target) {
                target.classList.toggle("-error", !result.isSuccess);
                target.classList.toggle("-success", result.isSuccess);
                let errorArea = target.querySelector(".SettingMain__settingItemErrorMessage");
                if (!errorArea)
                    return;
                errorArea.innerText = result.message;
            }
        }
        else if (result.target_name === "disclosure_scope" || result.target_name === "community_state_disclosure") {
            let target = document.querySelector(".clientRow[data-updatetype='" + result.target_name + "']");
            if (target) {
                if (!target)
                    return;
                target.classList.toggle("-error", !result.isSuccess);
                let errorMsgObj = target.querySelector(".SettingMain__settingItemErrorMessage");
                errorMsgObj.innerText = result.message;
                setSettingProfile(result.target_name, result.target_value);
            }
        }
        else if (result.target_name === "mailauth") {
            let target = document.querySelector("#setting__mailauth");
            if (target) {
                target.checked = result.target_value === "true";
            }
        }
    });
    document.querySelectorAll('.OptionPanel__itemTitle.-can-open').forEach(function (button) {
        button.addEventListener("click", showOptionRadio, false);
    });
    document.querySelector('#debug_login_code_copy').addEventListener("click", function (element) {
        copyClipborad("#debug_login_code", true);
        let msgCopyCmp = document.querySelector("#right_bottom_copytext");
        msgCopyCmp.classList.add("-copy");
        TweenMax.to(msgCopyCmp, .2, {
            opacity: "1",
            onComplete: function () {
                TweenMax.to(msgCopyCmp, .4, {
                    delay: 2.0,
                    opacity: "0",
                    onComplete: function () {
                        msgCopyCmp.classList.remove("-copy");
                    }
                });
            }
        });
    });
    var otherUserLoginObj = document.getElementById("other_user_login");
    if (otherUserLoginObj) {
        otherUserLoginObj.addEventListener("click", function (event) {
            event.preventDefault();
            let installBtn = document.getElementById("btn_content_install");
            clickedContentInstallBtn(installBtn.dataset.action, true);
        }, false);
    }
    document.getElementById("btn_content_install").addEventListener("click", function (event) {
        event.preventDefault();
        const action = this.dataset.action;
        clickedContentInstallBtn(action, false);
    }, false);
    document.querySelectorAll(".Operation__userName,.Operation__userIcon").forEach(function (ele) {
        ele.addEventListener("click", function () {
            closeAllModal("UserInfo");
            let UserInfo__backGround = document.querySelector('.UserInfo__backGround');
            let display = window.getComputedStyle(UserInfo__backGround, '').display;
            if (display == "none") {
                UserInfo__backGround.style.display = "block";
                TweenMax.to(UserInfo__backGround, .2, {
                    opacity: '1'
                });
            }
            else {
                TweenMax.to(UserInfo__backGround, .2, {
                    opacity: '0',
                    onComplete: () => {
                        UserInfo__backGround.style.display = "none";
                    }
                });
            }
        });
    });
    document.querySelector('.UserInfo__backGround').addEventListener("click", function (element) {
        if (element.target.classList.contains("UserInfo__backGround")) {
            hideUserInfoModal();
        }
    });
    document.querySelector('.CoinModal__backGround').addEventListener("click", function (element) {
        if (element.target.classList.contains("CoinModal__backGround")) {
            hideCoinShopModal();
        }
    });
    document.querySelector('.Achievement').addEventListener("click", function (e) {
        e.stopPropagation();
        hideAchievementModal();
    }, false);
    document.querySelector(".Operation__level").addEventListener("click", function () {
        var Achievement = document.querySelector('.Achievement');
        var display = window.getComputedStyle(Achievement, '').display;
        closeAllModal("Achievement");
        if (display == "none") {
            Achievement.style.display = "block";
            TweenMax.to(Achievement, .2, {
                opacity: "1"
            });
        }
        else {
            TweenMax.to(Achievement, .2, {
                opacity: "0",
                onComplete: function () {
                    Achievement.style.display = "none";
                }
            });
        }
    });
    document.querySelector(".Operation__coin").addEventListener("click", function () {
        closeAllModal("ShopCoin");
        let CoinModal = document.querySelector('.CoinModal__backGround');
        let display = window.getComputedStyle(CoinModal, '').display;
        CoinModal.style.display = "flex";
        TweenMax.to(CoinModal, .5, {
            opacity: "1"
        });
        var CoinModalDetails_1 = document.querySelector('.CoinModalDetails');
        if (CoinModalDetails_1.style.display == "flex") {
            CoinModalDetailsClose();
        }
        if (display == "none") {
            CoinModal.style.display = "flex";
            TweenMax.to(CoinModal, .5, {
                opacity: 1,
                ease: "Power4.easeInOut",
                onComplete: function () {
                }
            });
        }
        else {
            TweenMax.to(CoinModal, .5, {
                opacity: "0",
                onComplete: () => {
                    CoinModal.style.display = "none";
                }
            });
        }
    });
    window.addEventListener('resize', (e) => {
        clearInterval(timeOut);
        timeOut = setTimeout(function () {
            resize();
        }, 300);
    });
    try {
        ipcRenderer.on('update_content_list', function (event, arg) {
            console.log('update_content_list' + arg);
            var json = JSON.parse(arg);
            if (json != null) {
                let ele_pc_contents = document.getElementById('side_pc_contents');
                let ele_browser_contents = document.getElementById('side_browser_contents');
                contentlist_pc = getDictionary(json.pc_contents, "cuid");
                contentlist_browser = getDictionary(json.browser_contents, "cuid");
                decideSelectedCuid();
                updateContentList(ele_pc_contents, json.pc_contents, CONTENT_TYPE_PC);
                updateContentList(ele_browser_contents, json.browser_contents, CONTENT_TYPE_BROWSER);
                selectContent();
            }
        });
        ipcRenderer.on('update_content_tab', function (event, arg) {
            console.log('update_content_tab => ' + arg + " selected_cuid => " + selected_cuid);
            var json = JSON.parse(arg);
            if (json != null && selected_cuid == json.content_tab.cuid) {
                tablist = json.content_tab.tabs;
                delete json.content_tab.tabs;
                selected_content_data = json.content_tab;
                has_support_url = (json.content_tab.support_page_url ? true : false);
                setContentOptions(json.options);
                debugLoginCode(selected_content_data.debug_login_code);
                replaceUniqueOption(json.content_tab.cuid, json.content_tab.unique_configs);
                getNotificationVersion();
                updateContentTab();
            }
        });
        ipcRenderer.on('update_content_options', function (event, arg) {
            console.log('update_content_options => ' + arg + " selected_cuid => " + selected_cuid);
            var json = JSON.parse(arg);
            if (json != null && selected_cuid == json.cuid) {
                setContentOptions(json.options);
            }
        });
        ipcRenderer.on('update_content_state', function (event, arg) {
            console.log('update_content_state => ' + arg);
            try {
                var json = JSON.parse(arg);
                if (json.cuid === selected_cuid) {
                    updateContentState(json);
                    if (start_pending_cuid.length > 0) {
                        console.log('[update_content_state] start_pending_cuid : ' + start_pending_cuid);
                        if (start_pending_cuid[0] === selected_cuid) {
                            const btn = document.getElementById('btn_content_install');
                            console.log('[update_content_state] btn.dataset.action : ' + btn.dataset.action);
                            if (btn.dataset.action === 'bs_content_play') {
                                var object = { "cuid": selected_cuid, "from_shortcut": true, "other_user_login": false };
                                var itemjson = JSON.stringify(object);
                                console.log(itemjson);
                                ipcRenderer.send('bs_content_play', itemjson);
                            }
                            start_pending_cuid.splice(0, 1);
                        }
                        else {
                            sendContent(start_pending_cuid[0], TAB_TYPE_GAME_ID);
                        }
                    }
                }
            }
            catch (e) {
                console.log('[IPC (update_content_state) Error]' + e.message);
            }
        });
        ipcRenderer.on('update_profile', function (event, arg) {
            console.log('update_profile => ' + arg);
            var json = JSON.parse(arg);
            profile_data = json;
            updateProfile(profile_data);
        });
        ipcRenderer.on('update_account_content_list', function (event, arg) {
            console.log('update_account_content_list => ' + arg);
            var json = JSON.parse(arg);
            if (profile_data != null && json.account_content_list !== null) {
                profile_data.account_content_list = json.account_content_list;
                replaceGameList(json.account_content_list);
            }
        });
        ipcRenderer.on('update_achievement', function (event, arg) {
            console.log('update_achievement => ' + arg);
            var json = JSON.parse(arg);
            updateAchievement(json);
        });
        ipcRenderer.on('update_achievement_content', function (event, arg) {
            console.log('update_achievement_content => ' + arg);
            var json = JSON.parse(arg);
            updateAchievementContent(json);
        });
        ipcRenderer.on('update_shop', function (event, arg) {
            console.log('update_shop => ' + arg);
            var json = JSON.parse(arg);
            updateShop(json);
        });
        ipcRenderer.on('update_badge', function (event, arg) {
            console.log('update_badge => ' + arg);
            var json = JSON.parse(arg);
            updateBadge(json);
        });
        ipcRenderer.on('send_content', function (event, arg) {
            console.log('send_content => ' + arg);
            sendContent(arg, TAB_TYPE_GAME_ID);
        });
        ipcRenderer.on('send_content_news', function (event, arg) {
            console.log('send_content_news => ' + arg);
            sendContent(arg, TAB_TYPE_NEWS_ID);
        });
        ipcRenderer.on('content_login', function (event, arg) {
            console.log('content_login => ' + arg + ' selected_cuid => ' + selected_cuid);
            ipcRenderer.send('content_login_instruction', arg);
        });
        ipcRenderer.on('init_complete', function (event, arg) {
            console.log('init_complete => ' + arg);
            if (send_page === SEND_PAGE_SETTING) {
                ShowSetting(true, "option");
            }
        });
        ipcRenderer.on('content_maintenance', function (event, arg) {
            console.log('content_maintenance => ' + arg);
            if (selected_cuid === arg) {
                showContentMaintenance(arg);
            }
        });
        ipcRenderer.on('start_contents_shortcut', function (event, arg) {
            console.log('start_contents_shortcut => ' + arg);
            start_pending_cuid.push(arg);
            if (selected_content_type === CONTENT_TYPE_BROWSER) {
                sendContent(start_pending_cuid[0], TAB_TYPE_GAME_ID);
            }
            updateState();
        });
        ipcRenderer.on('update_active_friend_num', function (event, arg) {
            console.log('update_active_friend_num => ' + arg);
            updateActiveUser(arg);
        });
        ipcRenderer.on('content_install_start', function (event, arg) {
            console.log('content_install_start => ' + arg);
            let json = JSON.parse(arg);
            let cuid = json.cuid;
            if (selected_cuid == cuid) {
                updateContentState(json);
            }
            setTimeout(function () {
                if (waiting_installing.indexOf(cuid) < 0) {
                    waiting_installing.push(cuid);
                }
                if (selected_cuid === cuid) {
                    waiting_action_complete = CONTENT_ACTION_INSTALL;
                }
            }, 300);
        });
        ipcRenderer.on('content_update_start', function (event, arg) {
            console.log('content_update_start => ' + arg);
            let json = JSON.parse(arg);
            let cuid = json.cuid;
            if (selected_cuid == cuid) {
                updateContentState(json);
            }
        });
        ipcRenderer.on('content_uninstall_start', function (event, arg) {
            console.log('content_uninstall_start => ' + arg);
            setTimeout(function () {
                if (waiting_uninstalling.indexOf(arg) < 0) {
                    waiting_uninstalling.push(arg);
                }
                if (selected_cuid === arg) {
                    waiting_action_complete = CONTENT_ACTION_UNINSTALL;
                }
            }, 100);
        });
        ipcRenderer.on('act_window_max_result', function (event, arg) {
            console.log('[act_window_max_result] => ' + arg);
            changeMaxmizeIconHome(arg);
        });
        ipcRenderer.on('contents_operetion_error', function (event, arg) {
            console.log('contents_operetion_error');
            ipcRenderer.send('contents_operetion_error', arg);
        });
        ipcRenderer.on('content_download_error', function (event, arg) {
            console.log('content_download_error');
            ipcRenderer.send('content_download_error', arg);
        });
        loadOptionMenu();
        setOptionsEvent();
        console.log("shortcut_start => " + shortcut_start);
        if (shortcut_start) {
            shortcut_start = false;
            start_pending_cuid.push(selected_cuid);
        }
    }
    catch (e) {
        console.log("Error => " + e.message);
        ipcRenderer.send("javascript_error_msg", e.message);
    }
    window.addEventListener("message", function (event) {
        postMessageDecode(event, homePostMessageCallback);
    }, false);
    document.querySelector('.Web').addEventListener("load", function (e) {
        console.log("[ifream load] is_first_load => " + is_first_load.toString() + " is_load_content => " + is_load_content.toString());
        if (iframe_timeout_timer) {
            clearTimeout(iframe_timeout_timer);
            iframe_timeout_timer = null;
        }
        if (is_first_load)
            return false;
        setTimeout(function () {
            changeCoverOut();
            changeMenu('1', 'show');
            changeFooter('show');
        }, 500);
    });
    document.querySelector('.CoinModal__headerClose').addEventListener("click", function () {
        var CoinModal = document.querySelector('.CoinModal__backGround');
        var CoinModal__container = document.querySelector('.CoinModal__container');
        if (CoinModal__container.classList.contains('details')) {
            var CoinModal__titleList_1 = document.querySelector('.CoinModal__titleList');
            var CoinModal__productItems_1 = document.querySelector('.CoinModal__productItems');
            var CoinModalDetails_1 = document.querySelector('.CoinModalDetails');
            TweenMax.to(CoinModal, .5, {
                opacity: "0",
                onComplete: function () {
                    CoinModal.style.display = "none";
                    CoinModal__container.classList.remove('details');
                    TweenMax.to(CoinModalDetails_1, .5, {
                        opacity: "0",
                        onComplete: function () {
                            CoinModalDetails_1.style.display = "none";
                        }
                    });
                    CoinModal__titleList_1.style.display = "flex";
                    TweenMax.to(CoinModal__titleList_1, .5, {
                        opacity: "1"
                    });
                    CoinModal__productItems_1.style.display = "flex";
                    TweenMax.to(CoinModal__productItems_1, .5, {
                        opacity: "1"
                    });
                }
            });
        }
        else {
            var CoinModalDetails_1 = document.querySelector('.CoinModalDetails');
            if (CoinModalDetails_1.style.display == "flex") {
                CoinModalDetailsClose();
            }
            TweenMax.to(CoinModal, .5, {
                opacity: "0",
                onComplete: function () {
                    CoinModal.style.display = "none";
                }
            });
        }
    });
    document.querySelectorAll('.CoinModal__titleList li').forEach(function (button) {
        button.addEventListener("click", function () {
            document.querySelectorAll('.CoinModal__titleList li').forEach(function (CoinModal__titleList) {
                CoinModal__titleList.classList.remove('choice');
            });
            button.classList.add('choice');
            var filterName = button.getAttribute("data-filter");
            TweenMax.to(".CoinModal__product", .5, {
                opacity: "0",
                onComplete: function () {
                    document.querySelectorAll('.CoinModal__product').forEach(function (CoinModal__product) {
                        CoinModal__product.style.display = "none";
                    });
                    document.querySelectorAll('.CoinModal__product').forEach(function (CoinModal__product) {
                        if (CoinModal__product.classList.contains(filterName) || filterName == "all") {
                            CoinModal__product.style.display = "block";
                            TweenMax.to(CoinModal__product, .5, {
                                opacity: "1"
                            });
                        }
                    });
                }
            });
        });
    });
    var CoinModalDetails__back = document.querySelector('.CoinModalDetails__back');
    CoinModalDetails__back.addEventListener("click", function () {
        CoinModalDetailsClose();
    });
    var CoinModalDetails__exchangeButton = document.querySelector('.CoinModalDetails__exchangeButton');
    CoinModalDetails__exchangeButton.addEventListener("click", function () {
        var ConfirmModal = document.querySelector('.ConfirmModal');
        ConfirmModal.style.display = "flex";
        TweenMax.to(ConfirmModal, .5, {
            opacity: "1"
        });
    });
    var ConfirmModal__cancelButton = document.querySelector('.ConfirmModal__cancelButton');
    ConfirmModal__cancelButton.addEventListener("click", function () {
        var ConfirmModal = document.querySelector('.ConfirmModal');
        TweenMax.to(ConfirmModal, .5, {
            opacity: "0",
            onComplete: function () {
                ConfirmModal.style.display = "none";
            }
        });
    });
    document.querySelector('.ConfirmModal__deleteButton').addEventListener("click", function () {
        let CoinModalDetails__exchangeButton = document.querySelector('.CoinModalDetails__exchangeButton');
        ipcRenderer.send("exchange_item", CoinModalDetails__exchangeButton.dataset.shopid);
        var ConfirmModal = document.querySelector('.ConfirmModal');
        TweenMax.to(ConfirmModal, .5, {
            opacity: "0",
            onComplete: function () {
                ConfirmModal.style.display = "none";
            }
        });
    });
    ipcRenderer.on("show_exchange_dialog_reply", (event, arg) => {
        console.log('show_exchange_dialog_reply => ' + arg);
        let shopid = "shopid_" + arg;
        let CoinModal__product = document.querySelector('#' + shopid);
        CoinModal__product.classList.remove("unacquired");
        CoinModal__product.classList.add("acquire");
        CoinModalDetailsClose();
    });
    ipcRenderer.on("show_use_dialog_reply", (event, arg, arg2) => {
        console.log('show_use_dialog_reply => ' + arg);
        let shopid = "shopid_" + arg;
        const CoinModal__product = document.querySelector('#' + shopid);
        CoinModal__product.dataset.use = "true";
        if (arg2) {
            let shopid2 = "shopid_" + arg2;
            const CoinModal__product2 = document.querySelector('#' + shopid2);
            CoinModal__product2.dataset.use = "false";
        }
        CoinModalDetailsClose();
        reloadSelectContent();
    });
    ipcRenderer.on("show_unuse_dialog_reply", (event, arg) => {
        console.log('show_unuse_dialog_reply => ' + arg);
        let shopid = "shopid_" + arg;
        const CoinModal__product = document.querySelector('#' + shopid);
        CoinModal__product.dataset.use = "false";
        CoinModalDetailsClose();
        reloadSelectContent();
    });
    ipcRenderer.on('shop_social_badge', (event, arg) => {
        console.log('shop_social_badge => ' + arg);
        let disp = (arg === 'true' ? true : false);
        updateSocialBadge(disp);
    });
    initSetting();
});
//# sourceMappingURL=client_site.js.map