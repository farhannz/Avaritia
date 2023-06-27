$(function(){
  var modal = $('.install_wrapper'),
      installWindow = $('.install_window'),
      btnOpen = $(".topBtn_install"),
      btnClose = $(".btn_close_install");

  $(btnOpen).on('click', function() {
    modal.show();
  });

  $(modal).on('click', function(event) {
    if(!($(event.target).closest(installWindow).length)||($(event.target).closest(btnClose).length)){
      modal.hide();
    }
  });
});
