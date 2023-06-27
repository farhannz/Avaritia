const CHANNEL = {
    BROWSER_OPEN: "browser_open",
    GET_LOGIN_RESULT: "get_login_state",
    GET_CONTENT_LOGIN_RESULT: "get_content_login_state"
};
class PostMessageConnecter {
    constructor(event_type, event_data) {
        this.event_type = event_type;
        this.event_data = event_data;
    }
}
class IpcMessageRequestItem {
    constructor(cannel, msg) {
        this.cannel = cannel;
        this.msg = msg;
    }
}
function sendIframePostMessage(target_window, send_domain, event_type, data) {
    let post_data = new PostMessageConnecter(event_type, data);
    let wid = document.getElementById(target_window);
    wid.contentWindow.postMessage(post_data, send_domain);
}
function sendParentPostMessage(send_domain, event_type, data) {
    let post_data = new PostMessageConnecter(event_type, data);
    console.log('[message-child] send => event_type:' + event_type);
    window.parent.postMessage(post_data, send_domain);
}
function postMessageDecode(event, callback) {
    let data = event.data;
    console.log("[message] origin => " + event.origin + " event_type: " + data.event_type);
    data.event_origin = event.origin;
    callback(data);
}
//# sourceMappingURL=post_message.js.map