//=============================================================================
/// <reference path="../ts/jquery.d.ts"/>
//=============================================================================
function stop(): void {
  // alert("stop");

  $.get("command?cmd=gpio&par=stop",
    function(data) {}
  );
}
//=============================================================================
function forward(): void {
  // alert("forward");

  $.get("command?cmd=gpio&par=forward",
    function(data) {}
  );
}
//=============================================================================
function backward(): void {
  // alert("backward");

  $.get("command?cmd=gpio&par=backward",
    function(data) {}
  );
}
//=============================================================================
function left(): void {
  // alert("left");

  $.get("command?cmd=gpio&par=left",
    function(data) {}
  );
}
//=============================================================================
function right(): void {
  // alert("right");

  $.get("command?cmd=gpio&par=right",
    function(data) {}
  );
}
//=============================================================================
$(window).load(function() {
  // console.log("load");
  $("body").height($(window).height());
});
//=============================================================================
$(window).resize(function() {
  // console.log("resize");
  $("body").height($(window).height());
});
//=============================================================================
