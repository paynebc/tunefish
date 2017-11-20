
const os        = require('os');
const fs        = require('fs');
const moment    = require('moment');
const git       = require('simple-git')("../");

var version = "0.0";
try {
    version = fs.readFileSync("../VERSION_TF4").toString();
}
catch(e) { }
version = version.trim();

git.log({}, function(error, data) {
   if (error) {
      console.error(error);
   } else {
       git.status(function(error, status) {
           if (error) {
               console.error(error);
           } else {
               const buildnum = data.total;
               const date = moment(data.latest.date, 'YYYY-MM-DD HH:mm:ss Z').format('YYYYMMDD');
               const branch = status.current;
               const platform = os.platform() === 'darwin' ? 'macos' : os.platform();
               const arch = os.arch();

               const fullVersion = version + "." + buildnum + "-" + date + "-" + platform;
               console.log(fullVersion);
               fs.writeFileSync("../FULLVERSION_TF4", fullVersion);
           }
       });
   }
});