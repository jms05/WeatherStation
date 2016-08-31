CREATE PROCEDURE WeatherStationDB.getLastMonth()
BEGIN
    SELECT * FROM WeatherStationDB.record
    WHERE record.date >= DATE_ADD(LAST_DAY(DATE_SUB(NOW(), INTERVAL 2 MONTH)), INTERVAL 1 DAY) and
    record.date <= DATE_SUB(NOW(), INTERVAL 1 MONTH);
    
END

CREATE PROCEDURE WeatherStationDB.getLastYear()
BEGIN
    SELECT * FROM WeatherStationDB.record
    where record.date >= DATE_SUB(NOW(),INTERVAL 1 YEAR);
    
END

CREATE PROCEDURE WeatherStationDB.getLastWeek()
BEGIN
    SELECT * FROM WeatherStationDB.record
    WHERE record.date >= curdate() - INTERVAL DAYOFWEEK(curdate())+6 DAY
    AND record.date < curdate() - INTERVAL DAYOFWEEK(curdate())-1 DAY;
    
END