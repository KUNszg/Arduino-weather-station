#!/usr/bin/env node
'use strict';

const express = require('express');
const got = require('got');
const bodyParser = require('body-parser');
const mysql = require('mysql2');
const _ = require("lodash");
const config = require("./config");

class SQL {
    constructor(auth) {
        if (!auth) {
            auth = {
                host: config.db_host,
                user: config.db_user,
                password: config.db_pass,
                database: config.db_name,
                port: '/var/run/mysqld/mysqld.sock'
            }
        }

        this.auth = auth;
    }

    connect() {
        this.con = mysql.createConnection(this.auth);

        this.con.on('error', (err) => {
            if (err.fatal) {
                console.log(err)
            }
        });
    }

    async query(query, data = []) {
        try {
            const response = await this.con.promise().execute(mysql.format(query, data));
            const result = _.get(response, "0");
            this.con.unprepare(query);
            return result;
        } catch (err) {
            console.log(err)
        }
    }
}

const sql = new SQL();
const app = express();

sql.connect();

app.enable('trust proxy');
app.set('trust proxy', 1);
app.use(bodyParser.json());

app.put("/api/university-project", async (req, res) => {
    const {temperature, humudity, pm10, pm25, pm01, auth} = req.query;

    if (auth !== config.auth) {
        res.status(401);
        return;
    }

    await sql.query(
        "INSERT INTO weather_data (temperature, humidity, pm10, pm25, pm01) VALUES (?, ?, ? ,? ,?)",
        [temperature, humudity, pm10, pm25, pm01]);
});

app.listen(process.env.PORT || 8080, '0.0.0.0');