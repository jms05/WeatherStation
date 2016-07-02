CREATE PROCEDURE redordsDB.getLastMonth()
BEGIN
    SELECT * FROM redordsDB.record
    WHERE record.date >= DATE_ADD(LAST_DAY(DATE_SUB(NOW(), INTERVAL 2 MONTH)), INTERVAL 1 DAY) and
    record.date <= DATE_SUB(NOW(), INTERVAL 1 MONTH);
    
END


CREATE PROCEDURE redordsDB.getLastYear()
BEGIN
    SELECT * FROM redordsDB.record
    where record.date >= DATE_SUB(NOW(),INTERVAL 1 YEAR);
    
END

CREATE PROCEDURE redordsDB.getLastWeek()
BEGIN
    SELECT * FROM redordsDB.record
    WHERE record.date >= curdate() - INTERVAL DAYOFWEEK(curdate())+6 DAY
    AND record.date < curdate() - INTERVAL DAYOFWEEK(curdate())-1 DAY;
    
END