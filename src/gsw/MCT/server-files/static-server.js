var express = require('express');

function StaticServer() {
    var router = express.Router();

    router.use('/', express.static(__dirname + '/..'))
    //router.use('/scripts', express.static(__dirname + '/node_modules/openmct/dist/'));
    return router
}

module.exports = StaticServer;
