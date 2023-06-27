function setContentVolume() {
    if (content_volume <= 0)
        return;
    let dispVolume = convertSizeStringMb(content_volume);
    let volumeObj = document.querySelector(".GameInstallFunction__volumeValue");
    volumeObj.innerText = dispVolume;
}
function setInstallWIndowBackgroundColor(backgroundColor, fontColor) {
    const gameInstall = document.querySelectorAll('.GameInstall__container,.ProgressInfo__mask,.GameInstallFunction__directorySelector > input[type="text"],.GameInstallFunction__directorySelector > a');
    gameInstall.forEach(function (item) {
        item.style.background = backgroundColor;
    });
    const progressLabels = document.querySelectorAll('.GameInstallMainArea__Heading, .GameInstallFunction__directoryHeading, .GameInstallFunction__directorySelector > input[type="text"],.GameInstallFunction__directorySelector > a,.GameInstallFunction__volumeHeading, .GameInstallFunction__languageHeading,.GameInstallFunction__optionItem > label,.GameInstallFunction__optionItem > input[type=checkbox]:checked + label,.GameInstallFunction__languageValue,.GameInstallFunction__volumeValue');
    progressLabels.forEach(function (label) {
        label.style.color = fontColor;
    });
}
$(function () {
    modal_selector = ".GameInstall__container";
    setContentVolume();
    var installBtn = document.querySelector(".GameInstallAction__Button");
    installBtn.addEventListener("click", function (event) {
        console.log('install button clicked');
        var object = {};
        let form = document.getElementById("install_form");
        let formData = new FormData(form);
        formData.forEach(function (value, key) {
            object[key] = value;
        });
        let langFiles = [];
        object["language_files"] = langFiles;
        let autoUpdate = document.getElementById("AutoInstall");
        object["auto_update"] = autoUpdate.checked;
        let createShortcut = document.getElementById("Shortcut");
        object["create_shortcut"] = createShortcut.checked;
        var json = JSON.stringify(object);
        console.log(json);
        ipcRenderer.send("install_start", json);
    }, false);
    document.getElementById("c_select_directory").addEventListener("click", function (event) {
        event.preventDefault();
        let installDir = document.querySelector(".GameInstallFunction__directorySelector > input");
        ipcRenderer.send("select_directory", installDir.value);
    }, false);
    ipcRenderer.on("select-directory-reply", function (event, arg) {
        console.log("select-directory-reply => " + arg);
        if (arg) {
            let json = JSON.parse(arg);
            if (json.directory) {
                let installDir = document.querySelector(".GameInstallFunction__directorySelector > input");
                installDir.value = json.directory;
            }
            if (json.drive_volume) {
                let volumeDir = document.querySelector(".GameInstallFunction__discVolumeValue");
                volumeDir.innerText = json.drive_volume;
            }
        }
    });
    ipcRenderer.on("update_content_volume", function (event, arg) {
        console.log("update_content_volume => " + arg);
        var json = JSON.parse(arg);
        console.log("update_content_volume => cuid:" + CUID + " json.cuid:" + json.cuid + " json.total_size:" + json.total_size);
        if (json.cuid === CUID) {
            content_volume = json.total_size;
            setContentVolume();
            installBtn.classList.remove("disabled");
        }
    });
    ipcRenderer.on("parent_modal_clicked", function (event, arg) {
        console.log("parent_modal_clicked windowType => " + windowtype);
        windowActionClose(windowtype);
    });
    ipcRenderer.send("get_display_volume", CUID);
});
//# sourceMappingURL=install.js.map