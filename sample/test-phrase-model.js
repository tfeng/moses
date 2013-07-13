var path = require("path");
var Moses = require("moses").Moses;
var m = new Moses();

process.chdir(path.join(process.env.WORK, "sample-models"));
m.init("phrase-model/moses.ini", function(err) {
  if (err) {
    console.log("Error: " + err);
  } else {
    console.log(m.translate("das ist ein kleines haus\n"));
  }
});
console.log("Started.");
