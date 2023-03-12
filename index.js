#!/usr/bin/env node
'use strict';

const express = require('express');
const got = require('got');
const bodyParser = require('body-parser');
const mysql = require('mysql2');
const _ = require("lodash");
const config = require("./config");
const init = require("../kbot/lib/utils/connection");

const kb = new init.IRC();
kb.sqlConnect();

const app = express();

app.enable('trust proxy');
app.set('trust proxy', 1);
app.use(bodyParser.json());

app.get("/api/university-project", async (req, res) => {
    const {temperature, humidity, pm10, pm25, pm01, auth} = req.query;

    if (auth !== config.auth) {
        res.sendStatus(401);
        return;
    }

    await kb.query(
        "INSERT INTO weather_data (temperature, humidity, pm10, pm25, pm01) VALUES (?, ?, ? ,? ,?)",
        [temperature, humidity, pm10, pm25, pm01]);

   res.sendStatus(200);
});

app.listen(process.env.PORT || 8090, '0.0.0.0');
