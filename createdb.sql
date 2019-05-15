PRAGMA encoding="UTF-8";
PRAGMA foreign_keys = ON;

create table MORPHOLOGY(
wordform varchar(256),
group_id smallint,
feature_nr smallint,
feature varchar(128),
PRIMARY KEY(wordform,group_id,feature_nr)
);
