var coverWrapped;
function getTextWidth(text, font) {
    const canvas = document.createElement("canvas");
    const context = canvas.getContext("2d");
    context.font = font;
    const metrics = context.measureText(text);
    return metrics.width;
}
function scrollNews() {
    setTimeout(function () {
        sliderNews();
    }, 2000);
    setPosition();
}
function resize() {
    setPosition();
    changeMessage('hidden');
    setTimeout(function () {
        changeMessage('show');
    }, 1000);
}
function sliderNews() {
    let slideIndex = 0;
    slider();
    function slider() {
        let i;
        let items = document.querySelectorAll(".Notification__message");
        for (i = 0; i < items.length; i++) {
            if (!items[i].classList.contains('stop')) {
                items[i].classList.remove("slide");
                items[i].classList.remove("slideOut");
                items[i].classList.add("slideOut");
            }
        }
        slideIndex++;
        if (slideIndex > items.length) {
            slideIndex = 1;
        }
        let itemCurrent = items[slideIndex - 1];
        if (itemCurrent && !itemCurrent.classList.contains('stop')) {
            let sliderLength = itemCurrent.querySelector('.Notification__messageLink').clientWidth;
            let messageLength = itemCurrent.clientWidth;
            itemCurrent.classList.remove("slideOut");
            itemCurrent.classList.add("slide");
            setTimeout(() => {
                itemCurrent.querySelector('.Notification__messageLink').classList.remove('scroll');
            }, 9000);
            if (sliderLength > messageLength) {
                setTimeout(slider, 7000);
                setTimeout(() => {
                    itemCurrent.querySelector('.Notification__messageLink').classList.add('scroll');
                }, 4000);
            }
            else {
                setTimeout(slider, 5000);
            }
        }
    }
}
function setPosition() {
    const sliders = document.querySelectorAll('.Notification__messageLink');
    sliders.forEach(function (slider) {
        const sliderWidth = slider.clientWidth;
        const messageWidth = document.querySelector('.Notification__message').clientWidth;
        slider.style.right = `${messageWidth - sliderWidth - 42}px`;
    });
}
function changeMessage(className = "hidden") {
    let messages = document.querySelectorAll(".Notification__message");
    messages.forEach((message) => {
        message.classList.remove('hidden');
        message.classList.remove('show');
        message.classList.add(className);
    });
}
function hideUserInfoModal() {
    var toggle_contents = document.querySelector('.UserInfo__backGround');
    toggle_contents.classList.remove('-show');
    toggle_contents.style.display = "none";
}
function hideAchievementModal() {
    var Achievement = document.querySelector('.Achievement');
    TweenMax.to(".Achievement", .2, {
        opacity: "0",
        onComplete: function () {
            Achievement.style.display = "none";
        }
    });
}
function hideCoinShopModal() {
    var CoinModal__backGround = document.querySelector('.CoinModal__backGround');
    TweenMax.to(CoinModal__backGround, .5, {
        opacity: "0",
        onComplete: function () {
            CoinModal__backGround.style.display = "none";
        }
    });
}
function closeAllModal(exceptModal) {
    if (exceptModal != "Achievement") {
        hideAchievementModal();
    }
    if (exceptModal != "ShopCoin") {
        hideCoinShopModal();
    }
    if (exceptModal != "UserInfo") {
        hideUserInfoModal();
    }
    if (exceptModal != "Setting") {
        ShowSetting(false, "user");
    }
}
function changeFontColorSet(color) {
    changeColorMenuItems(color);
    changeColorGameItems(color);
}
function changeColorGameItems(color) {
    const gameItems = document.querySelectorAll('.GameItem__info,.SideMenu__appVersion');
    gameItems.forEach(function (gameItem) {
        gameItem.style.color = color;
    });
}
function changeColorMenuItems(color) {
    const menuItems = document.querySelectorAll('.MainMenu__item');
    menuItems.forEach(function (menuItem) {
        const imageItem = menuItem.querySelector('img');
        menuItem.style.color = color;
        color == 'black' ? imageItem.style.filter = "brightness(0%)" : imageItem.style.removeProperty('filter');
    });
}
function changeColorOperationItems(color) {
    const operationItems = document.querySelectorAll('.Operation__item,.WindowControl__maxmizeButton,.WindowControl__closeButton,.WindowControl__minimizeButton');
    operationItems.forEach(function (operationItem) {
        if (operationItem.querySelector('img')) {
            if (color == 'black') {
                operationItem.querySelector('img').style.filter = "brightness(0%)";
            }
            else {
                operationItem.querySelector('img').style.removeProperty('filter');
            }
        }
        if (color == 'black') {
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
        operationItem.style.color = color;
    });
}
function changeBackGround(backgroundStart, backgroundEnd, timeOut) {
    const mainbg = document.querySelector('.SideMenu');
    mainbg.style.background = backgroundStart;
    mainbg.classList.add('fade');
    mainbg.style.background = backgroundEnd;
}
function changeBackgroundMainMenu(background, timeOut) {
    const header = document.querySelector('.Header');
    const mainMenu = document.querySelector('.MainMenu');
    const mainMenuCut = document.querySelector('.Footer__cut');
    console.log("menuChange:" + background);
    console.log(mainMenuCut);
    setTimeout(function () {
        mainMenu.classList.add('fade');
        mainMenu.style.background = background;
        mainMenuCut.style.borderBottomColor = background;
    }, timeOut);
}
function changeMenu(opacity, className = 'hide') {
    const menuItems = document.querySelectorAll('.MainMenu .MainMenu__item');
    const actionInstall = document.querySelector('.Action__install .Action__item');
    const actionOption = document.querySelector('.Action__option .Action__item');
    const optionPanel = document.querySelector('.OptionPanel');
    optionPanel.classList.remove('rollup');
    actionInstall.classList.add('fade');
    actionOption.classList.add('fade');
    actionInstall.style.opacity = opacity;
    actionOption.style.opacity = opacity;
    menuItems.forEach(function (menuItem, index) {
        menuItem.classList.remove('show', 'hide', '-current');
        menuItem.classList.add(className);
        if (selected_tab_idx === index && className == 'show') {
            menuItem.classList.add('-current');
        }
    });
}
function changeFooter(className = 'hide') {
    const footer = document.querySelector('.Footer');
    const web = document.querySelector('.Web');
    web.classList.remove('hide', 'show');
    web.classList.add(className);
    footer.classList.remove('hide', 'show');
    footer.classList.add(className);
}
function changeIframe(className, zindex) {
    const iframe = document.querySelector(className);
    iframe.style.zIndex = zindex;
}
function loadIframe(className, dataHref) {
    const iframe = document.querySelector(className);
    iframe.src = dataHref;
    iframe.style.zIndex = "-1";
}
function showCartainLink(link) {
    TweenMax.set('.GameCover__container--sub--elm span', { opacity: 1 });
    document.querySelector('.GameCover__container').classList.add('gameContents');
    var mainbg = document.querySelectorAll('.GameCover__container--sub--elm span');
    for (var i = 0; i < mainbg.length; i++) {
        var bg = mainbg[i];
        bg.style.background = selected_content_data['background_color'];
    }
    console.log("main menu animation");
    TweenMax.fromTo('.Web', .2, {
        x: 0
    }, {
        x: 96,
        ease: "Power4.easeIn",
        onComplete: function () {
        }
    });
    TweenMax.fromTo('.GameCover__container--sub--elm span', .4, {
        x: '-100%'
    }, {
        x: '0%',
        ease: "Power4.easeIn",
        onComplete: function () {
            loadIframe('.Web', link);
            changeIframe('.Web', '0');
        }
    });
}
function removePreload() {
    TweenMax.to('.Preload', 1, { opacity: 0, zIndex: 0, ease: "Power2.easeOut" });
}
function fadeOut(node, duration) {
    node.style.opacity = '1';
    var start = performance.now();
    requestAnimationFrame(function tick(timestamp) {
        var easing = (timestamp - start) / duration;
        node.style.opacity = (Math.max(1 - easing, 0)).toString();
        if (easing < 1) {
            requestAnimationFrame(tick);
        }
        else {
            node.style.opacity = '';
            node.style.display = 'none';
        }
    });
}
function loadOptionMenu() {
    document.querySelector('.Action__option').addEventListener("click", function () {
        showOptionPanel(null);
    });
    document.addEventListener('click', function (element) {
        if (!element.target.closest('.OptionPanel') && !element.target.closest('.Action__option')) {
            let optionPanel = document.querySelector('.OptionPanel');
            let bkPanel = document.querySelector('.OptionPanel__bk');
            if (optionPanel.classList.contains("rollup")) {
                showOptionPanel(false);
            }
        }
    }, true);
}
function showOptionPanel(open) {
    var optionPanel = document.querySelector('.OptionPanel');
    let bkPanel = document.querySelector('.OptionPanel__bk');
    if (open === null) {
        open = !(optionPanel.classList.contains("rollup"));
    }
    if (open) {
        if (!optionPanel.classList.contains("rollup")) {
            bkPanel.style.display = 'flex';
            optionPanel.classList.remove('close');
            optionPanel.classList.add('rollup');
            optionPanel.style.display = "block";
            TweenMax.to(optionPanel, .2, {
                bottom: 95,
                ease: "Power4.easeInOut",
                onComplete: function () {
                }
            });
        }
    }
    else {
        if (optionPanel.classList.contains("rollup")) {
            bkPanel.style.display = 'none';
            optionPanel.classList.remove('rollup');
            optionPanel.classList.add('close');
            let radioContents = document.querySelectorAll('.OptionPanel__contents.-show');
            radioContents.forEach(function (ele) {
                ele.classList.remove('-show');
                let prevEle = ele.previousElementSibling;
                if (prevEle && prevEle.classList.contains('active')) {
                    prevEle.classList.remove('active');
                }
            });
            TweenMax.to(optionPanel, .2, {
                bottom: -256,
                ease: "Power4.easeInOut",
                onComplete: function () {
                    setTimeout(function () {
                        optionPanel.style.display = "none";
                    }, 300);
                }
            });
        }
    }
}
function changeCover(game) {
    coverWrapped = true;
    var footerBtn = document.querySelector('.Footer');
    footerBtn.classList.add('pointer_events_none');
    document.querySelector('.GameCover__container').classList.remove('gameContents');
    document.querySelector('.loading').classList.add('active');
    TweenMax.fromTo('.loading', .8, {
        y: -15,
        opacity: 0,
    }, {
        y: 0,
        opacity: 1,
        ease: "Power4.easeIn",
        onComplete: function () {
        }
    });
    TweenMax.to(".Web", 2, {
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
            changeMenu('0');
            changeBackGround('rgba(255,255,255,0)', game['background_color'], 0);
            changeBackgroundMainMenu(game['background_color'], 0);
            var mainbg = document.querySelector('.GameCover__container--bottom');
            mainbg.classList.add('active');
            mainbg.style.background = game['background_color'];
            changeFontColorSet(game['font_color']);
            var ProgressInfo = document.querySelector('.ProgressInfo');
            ProgressInfo.style.display = 'none';
            TweenMax.to('.ProgressInfo', .3, {
                y: 40,
                ease: "Power4.easeOut"
            });
        }
    });
}
function changeCoverOut() {
    var mainbg = document.querySelector('.GameCover__container--bottom');
    mainbg.classList.remove('active');
    var footerBtn = document.querySelector('.Footer');
    if (!is_load_content) {
        if (coverWrapped == true) {
            TweenMax.fromTo('.Web', .7, {
                x: -96
            }, {
                x: 0,
                ease: "Power4.easeOut",
                onComplete: function () {
                }
            });
            coverWrapped = false;
        }
        TweenMax.fromTo('.GameCover__container--sub--elm span', .7, {
            x: '0%',
        }, {
            x: '100%',
            ease: "Power4.easeOut",
            onComplete: function () {
                TweenMax.set('.GameCover__container--sub--elm span', { opacity: 0 });
            }
        });
    }
    else {
        TweenMax.fromTo('.Web', .7, {
            y: 96
        }, {
            y: 0,
            ease: 'Power4.easeOut',
            onComplete: function () {
            }
        });
        TweenMax.fromTo('.GameCover__container--bottom', .7, {
            y: '0%'
        }, {
            y: '-100%',
            ease: "Power4.easeOut",
            onComplete: function () {
            }
        });
    }
    TweenMax.fromTo('.loading', .5, {
        y: 0,
        opacity: 1,
    }, {
        y: -15,
        opacity: 0,
        ease: "Power4.easeOut",
        onComplete: function () {
            document.querySelector('.loading').classList.remove('active');
            footerBtn.classList.remove('pointer_events_none');
            is_loading = false;
        }
    });
}
function preCallProgressBar() {
    TweenMax.to(".Web", .3, {
        delay: .7,
        height: "calc(100vh - 185px)",
        ease: "Power4.easeOut",
        onComplete: function () {
            TweenMax.to(".Web", 1, {
                delay: ".8",
                height: "calc(100vh - 145px)"
            });
        }
    });
}
//# sourceMappingURL=index.js.map