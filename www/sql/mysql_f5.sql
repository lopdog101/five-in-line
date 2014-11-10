;mysql -u root
create database f5;
grant all on f5 to ''@'localhost';
grant all on f5.* to ''@'localhost';

CREATE TABLE s13(
  k varbinary(39) NOT NULL DEFAULT '',
  
  state integer NOT NULL default 0,
  wins_count BIGINT NOT NULL default 0,
  fails_count BIGINT NOT NULL default 0,

  neitrals varbinary(800) NOT NULL,
  solved_wins varbinary(800) NOT NULL,
  solved_fails varbinary(800) NOT NULL,
  tree_wins varbinary(800) NOT NULL,
  tree_fails varbinary(800) NOT NULL,

  PRIMARY KEY (`k`) USING HASH
  ) ENGINE=MyISAM;


CREATE TABLE f5(k varbinary(600) NOT NULL DEFAULT '',d varbinary(60000) NOT NULL,PRIMARY KEY (`k`) USING HASH) ENGINE=MyISAM;


select hex(k) from f5 order by k>x'F70001F90002FB0101FD0302FF0002FF0101FF0201000001000101000202010102010201020202' order by k limit 1;