-- 데이터베이스 생성
--CREATE DATABASE AccountDB
--GO

USE AccountDB;
GO

-- 계정 테이블
DROP   TABLE dbo.Account
CREATE TABLE dbo.Account
(
    AccountID   BIGINT IDENTITY(1,1)    NOT NULL,
    UserID      NVARCHAR(64)            NOT NULL,
    UserPass    NVARCHAR(128)           NOT NULL,
    UserNick    NVARCHAR(64)            NOT NULL,
    CreateDate  DATETIME2(3)            NOT NULL,

    CONSTRAINT PK_Account PRIMARY KEY (AccountID)
);
GO

CREATE UNIQUE INDEX IX_Account_UserID ON dbo.Account(UserID)
CREATE UNIQUE INDEX IX_Account_UserNick ON dbo.Account(UserNick)
CREATE INDEX IX_Account_CreateDate ON dbo.Account(CreateDate)
GO

-- 세션키 테이블
DROP   TABLE dbo.AccountSessionKey
CREATE TABLE dbo.AccountSessionKey
(
    AccountID       BIGINT          NOT NULL,
    Sessionkey      NVARCHAR(64)    NULL,

    CONSTRAINT PK_AccountSessionKey PRIMARY KEY (AccountID)
);
GO

-- 상태 테이블
DROP   TABLE dbo.AccountStatus
CREATE TABLE dbo.AccountStatus
(
    AccountID       BIGINT  NOT NULL,
    Status          INT     NOT NULL

    CONSTRAINT DF_AccountStatus_Status DEFAULT (0),
    CONSTRAINT PK_AccountStatus PRIMARY KEY (AccountID)
);
GO

-- 화이트 IP 테이블
DROP   TABLE dbo.WhiteIP
CREATE TABLE dbo.WhiteIP
(
    NO BIGINT IDENTITY(1,1)     NOT NULL,
    IP NVARCHAR(32)             NOT NULL,

    CONSTRAINT PK_WhiteIP PRIMARY KEY (NO)
);
GO

-- View 생성
--DROP   VIEW dbo.v_Account
CREATE VIEW dbo.v_Account
AS
SELECT
    a.AccountID,
    a.UserID,
    a.UserNick,
    b.Sessionkey,
    c.Status
FROM dbo.Account AS a
    LEFT JOIN dbo.AccountSessionKey AS b
        ON a.AccountID = b.AccountID
    LEFT JOIN dbo.Accountstatus AS c
        ON a.AccountID = c.AccountID
GO

-- 더미 데이터 생성
DECLARE @i INT = 1
WHILE @i <= 100000
BEGIN
    INSERT INTO [AccountDB].[dbo].[Account] (UserID, UserPass, UserNick, CreateDate)
    VALUES
    (
        N'ID_' + CAST(@i AS NVARCHAR(10)),
        CONVERT(
            NVARCHAR(128),
            HASHBYTES(
                'SHA2_512',
                CONVERT(NVARCHAR(36), NEWID()) +
                CONVERT(NVARCHAR(36), NEWID())
            ),
            2
        ),
        N'NICK_' + CAST(@i AS NVARCHAR(10)),
        DATEADD(
            SECOND,
            ABS(CHECKSUM(NEWID())) %
                DATEDIFF(SECOND, '2024-01-01', GETDATE()),
            '2024-01-01'
        )
    )

    INSERT INTO [AccountDB].[dbo].[AccountSessionKey] (AccountID, SessionKey) VALUES (@i, NULL)
    INSERT INTO [AccountDB].[dbo].[AccountStatus] (AccountID, Status) VALUES (@i, 0)

    SET @i += 1
END