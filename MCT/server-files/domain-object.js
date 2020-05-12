class DomainObject {
  function notify (point,listener){
    listener.forEach(function (l) {
        l(point);
  });
}
