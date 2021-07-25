
const e = require('express');
var express = require('express');

function HistoryServer(spacecraft) {
    var router = express.Router();

    router.get('/:pointId', async function (req, res) {
        var start = +req.query.start;
        var end = +req.query.end;
        var ids = req.params.pointId.split(',');

        var response = await spacecraft.ReceiveTelemetry(start, end, ids[0]);
        res.status(200).json(response).end();
    });

    return router;
}

module.exports = HistoryServer;
