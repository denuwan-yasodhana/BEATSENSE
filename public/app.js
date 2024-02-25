//getting reference to the data we want
var dataRef1 = database.ref('test/body/BPM');
var dataRef2 = database.ref('test/body/Body Temperature');
var dataRef3 = database.ref('test/room/Room Temperature');

//fetch the data
dataRef1.on('value', function(getdata1){
  var bpm = getdata1.val();
  document.getElementById('bpm').innerHTML = bpm;
})

dataRef2.on('value', function(getdata2){
  var bTemp = getdata2.val();
  document.getElementById('bTemp').innerHTML = bTemp + "&#8451;";
})

dataRef3.on('value', function(getdata3){
  var rTemp = getdata3.val();
  document.getElementById('rTemp').innerHTML = rTemp + "&#8451;";
})