BEGIN TRANSACTION;
ANALYZE sqlite_master;
CREATE TABLE ChildServices
(
	ParentID            		Integer not null,
	ChildID				Integer not null,
	MetaDataID			Integer not null,

	primary key (ParentID, ChildID, MetaDataID)
);
CREATE TABLE ServiceMetaData 
(
	ID			Integer primary key AUTOINCREMENT not null,
	ServiceID		Integer not null,
	MetaDataID 		Integer  not null,
	MetaDataValue     	Text,
	MetaDataDisplay		Text

);
DELETE FROM sqlite_sequence;
CREATE TABLE ServiceKeywordMetaData 
(
	ID			Integer primary key AUTOINCREMENT not null,
	ServiceID		Integer not null,
	MetaDataID 		Integer not null,
	MetaDataValue		Text COLLATE NOCASE
);
CREATE TABLE ServiceNumericMetaData 
(
	ID			Integer primary key AUTOINCREMENT not null,
	ServiceID		Integer not null,
	MetaDataID 		Integer not null,
	MetaDataValue		Integer not null
);
CREATE TABLE MailSummary
(
	ID		Integer primary key AUTOINCREMENT not null,
	MailApp		Integer not null,
	MailType	Integer not null,
	FileName	Text not null,
	Path		Text not null,
	UriPrefix	Text,
	NeedsChecking	Integer default 0,
	MailCount	Integer,
	JunkCount	Integer,
	DeleteCount	Integer,
	Offset		Integer,
	LastOffset	Integer,
	MTime		integer,

	unique (Path)
);
CREATE TABLE JunkMail
(
	UID			integer not null,
	SummaryID		Integer not null,

	primary key (UID, SummaryID)
);
CREATE INDEX ChildServicesIndex1 ON ChildServices (ChildID);
CREATE INDEX ServiceMetaDataIndex1 ON ServiceMetaData (ServiceID);
CREATE INDEX ServiceMetaDataIndex2 ON ServiceMetaData (MetaDataID);
CREATE INDEX ServiceKeywordMetaDataIndex1 ON ServiceKeywordMetaData (MetaDataID, MetaDataValue);
CREATE INDEX ServiceKeywordMetaDataIndex2 ON ServiceKeywordMetaData (ServiceID);
CREATE INDEX ServiceNumericMetaDataIndex1 ON ServiceNumericMetaData (MetaDataID, MetaDataValue);
CREATE INDEX ServiceNumericMetaDataIndex2 ON ServiceNumericMetaData (ServiceID);
COMMIT;
