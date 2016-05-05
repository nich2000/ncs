/// <reference path="../ts/jquery.d.ts"/>
function stop() {
    // alert("stop");
    $.get("command?cmd=gpio&par=stop", function (data) { });
}
function forward() {
    // alert("forward");
    $.get("command?cmd=gpio&par=forward", function (data) { });
}
function backward() {
    // alert("backward");
    $.get("command?cmd=gpio&par=backward", function (data) { });
}
function left() {
    // alert("left");
    $.get("command?cmd=gpio&par=left", function (data) { });
}
function right() {
    // alert("right");
    $.get("command?cmd=gpio&par=right", function (data) { });
}
$(window).load(function () {
    $("body").height($(window).height());
});
$(window).resize(function () {
    $("body").height($(window).height());
});
//# sourceMappingURL=script.js.map