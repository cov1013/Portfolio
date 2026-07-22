CREATE DATABASE LogDB
GO

USE LogDB
GO

-- 게임 로그 테이블
CREATE TABLE dbo.GameLog
(
    NO              BIGINT IDENTITY(1,1)    NOT NULL,
    AddDate         DATETIME2(3)            NOT NULL CONSTRAINT DF_GameLog_EventDate DEFAULT(SYSDATETIME()),
    ServerID        INT                     NOT NULL,
    Type            INT                     NOT NULL,
    Code            INT                     NOT NULL,
    AccountID       BIGINT                  NOT NULL,
    CharacterID     INT                     NOT NULL,
    Param1          BIGINT                  NULL,
    Param2          BIGINT                  NULL,
    Param3          BIGINT                  NULL,
    Param4          BIGINT                  NULL,
    Param5          BIGINT                  NULL,
    Param6          BIGINT                  NULL,
    Param7          BIGINT                  NULL,
    Param8          BIGINT                  NULL,
    ParamStr        NVARCHAR(512)           NULL,

    CONSTRAINT PK_GameLog PRIMARY KEY CLUSTERED (AddDate, NO)
)
GO

CREATE INDEX IX_GameLog_CharacterID_AddDate ON dbo.GameLog(CharacterID, AddDate DESC) INCLUDE (Code, AccountID, Param1, Param2, Param3, Param4)
GO

-- 게임 로그 코드 테이블
CREATE TABLE dbo.GameLogCode
(
    Type            INT             NOT NULL,
    Code            INT             NOT NULL,
    Description     NVARCHAR(500)   NOT NULL,
    CreateDate      DATETIME2(3)    NOT NULL CONSTRAINT DF_GameLogEventCode_CreateDate DEFAULT(SYSDATETIME()),

    CONSTRAINT PK_GameLogCode PRIMARY KEY (Type, Code)
)
GO

INSERT INTO dbo.GameLogCode
(
    Type,
    Code,
    Description
)
VALUES
-- 접속 로그
(1, 1001, N'게임 서버 로그인 성공'),
(1, 1002, N'게임 서버 로그아웃'),

-- 전투 로그
(2, 2001, N'몬스터 처치'),

-- 재화 로그
(3, 3001, N'몬스터 처치 보상으로 골드 획득'),
(3, 3002, N'퀘스트 완료 보상으로 골드 획득'),
(3, 3003, N'상점에서 아이템 구매로 골드 차감'),

-- 경험치 로그
(4, 4001, N'몬스터 처치 보상으로 경험치 획득'),
(4, 4002, N'퀘스트 완료 보상으로 경험치 획득'),

-- 아이템 로그
(5, 5001, N'몬스터 드롭 아이템 획득'),
(5, 5002, N'상점에서 아이템 구매'),
(5, 5003, N'아이템 사용'),
(5, 5004, N'아이템 삭제'),

-- 캐릭터 로그
(6, 6001, N'캐릭터 생성'),
(6, 6002, N'캐릭터 레벨업');
GO