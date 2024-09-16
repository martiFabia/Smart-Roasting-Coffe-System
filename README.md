<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
</head>
<body>

<h1>Smart Coffee Roasting System</h1>

<h2>Overview</h2>

<p>This project is part of the <strong>Industry 4.0 framework</strong>, leveraging advanced technologies to develop a smart solution for monitoring and controlling the coffee roasting process. Our goal is to ensure <strong>consistent product quality</strong> without increasing costs, while improving operational efficiency through automation and real-time data monitoring.</p>

<h3>Key Features:</h3>
<ul>
    <li><strong>Real-time Monitoring</strong>: Integrated sensors for humidity, temperature, and CO2 levels using the MQTT protocol.</li>
    <li><strong>Automated Control</strong>: Actuators for regulating temperature, ventilation, and an alert system, communicating via the CoAP protocol.</li>
    <li><strong>Remote Control</strong>: A mobile app for real-time parameter monitoring and adjustments.</li>
    <li><strong>Data Management</strong>: MySQL database to store historical data for analysis and quality assurance.</li>
    <li><strong>Industry 4.0 Integration</strong>: IoT, automation, and smart control to digitize and optimize the roasting process.</li>
</ul>

<h2>Components</h2>
<ol>
    <li><strong>MQTT network</strong>:
        <ul>
            <li>Humidity Sensor</li>
            <li>Temperature Sensor</li>
            <li>CO2 Sensor</li>
            <li>MQTT broker </li>
            <li>MQTT Collector </li>
        </ul>
    </li>
    <li><strong>CoAP network</strong>:
        <ul>
            <li>Temperature Regulator</li>
            <li>Fan</li>
            <li>Alert System</li>
            <li>CoAP Registration server</li>
        </ul>
    </li>
    <li><strong>Control System</strong>: Automation logic to maintain optimal conditions based on real-time sensor data.</li>
    <li><strong>Database</strong>: MySQL for data logging and analysis.</li>
    <li><strong>Remote Monitoring</strong>: Mobile application for remote access and control of roasting parameters.</li>
</ol>

<h2>Usage</h2>
<ol>
    <li><strong>Start the system</strong>: Ensure all sensors (MQTT) and actuators (CoAP) are connected.</li>
    <li><strong>Monitor the roasting process</strong>: Use the remote control app to view real-time data and the current status of the available actuators</li>
    <li><strong>Adjust parameters</strong>: Modify the temperature, humidity, or CO2 levels based on roasting profiles.</li>
    <li><strong>Data Analysis</strong>: View and analyze historical data stored in the MySQL database.</li>
</ol>

</body>
</html>
