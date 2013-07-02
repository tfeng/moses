"use strict";

var Moses = require('bindings')('Moses.node');

exports.init = Moses.init;
exports.translate = Moses.translate;