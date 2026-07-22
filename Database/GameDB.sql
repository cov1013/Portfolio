-- 데이터베이스 생성
CREATE DATABASE GameDB
GO

USE GameDB
GO

-- 캐릭터 테이블
DROP   TABLE dbo.Character
CREATE TABLE dbo.[Character]
(
    CharacterID     BIGINT IDENTITY(1,1)    NOT NULL,
    AccountID       BIGINT                  NOT NULL,
    CharacterName   NVARCHAR(64)            NOT NULL,
    CharacterStatus INT                     NOT NULL CONSTRAINT DF_Character_Status DEFAULT(1),
    CreateDate      DATETIME2(3)            NOT NULL CONSTRAINT DF_Character_CreateDate DEFAULT(SYSDATETIME()),

    CONSTRAINT PK_Character PRIMARY KEY (CharacterID)
)
CREATE UNIQUE INDEX UX_Character_CharacterName ON dbo.[Character](CharacterName);
CREATE INDEX IX_Character_AccountID ON dbo.[Character](AccountID) INCLUDE (CharacterName, CreateDate);
GO

-- 캐릭터 스탯 테이블
CREATE TABLE dbo.CharacterStat
(
    CharacterID     BIGINT       NOT NULL,
    CharacterLevel  INT          NOT NULL CONSTRAINT DF_CharacterStat_Level DEFAULT(1),
    CharacterStr    INT          NOT NULL CONSTRAINT DF_CharacterStat_Str DEFAULT(1),
    CharacterDex    INT          NOT NULL CONSTRAINT DF_CharacterStat_Dex DEFAULT(1),
    CharacterInt    INT          NOT NULL CONSTRAINT DF_CharacterStat_Int DEFAULT(1),
    CharacterLuck   INT          NOT NULL CONSTRAINT DF_CharacterStat_Luck DEFAULT(1),
    UpdateDate      DATETIME2(3) NOT NULL CONSTRAINT DF_CharacterStat_UpdateDate DEFAULT(SYSDATETIME()),

    CONSTRAINT PK_CharacterStat PRIMARY KEY (CharacterID),
    CONSTRAINT FK_CharacterStat_Character FOREIGN KEY (CharacterID) REFERENCES dbo.[Character](CharacterID) ON DELETE CASCADE
)
GO