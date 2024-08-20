-- Drop the database if it exists
DROP DATABASE IF EXISTS RoastingMachine;

-- Create the database
CREATE DATABASE RoastingMachine;

-- Use the created database
USE RoastingMachine;

-- Drop the sensor table if it exists
DROP TABLE IF EXISTS dataSensed;

-- Create the dataSensed table
CREATE TABLE dataSensed (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    value INT,
    type VARCHAR(256),
    interval INT
);

-- Drop the actuators table if it exists
DROP TABLE IF EXISTS actuators;

-- Create the actuators table
CREATE TABLE actuators (
    ip VARCHAR(256) PRIMARY KEY,
    resource VARCHAR(256) NOT NULL,
    status VARCHAR(256) 
);

