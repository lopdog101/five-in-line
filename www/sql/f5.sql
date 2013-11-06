
CREATE TABLE solve_stat(
    id          serial PRIMARY KEY,
    src_name    text NOT NULL default '',
	ip          text NOT NULL default '',
	root_name   text NOT NULL default '',
    log_date    date NOT NULL,
	solve_count integer NOT NULL default 0,
	unique(src_name,ip,log_date,root_name)
);

create index solve_stat_src_name_idx on solve_stat(src_name);
create index solve_stat_ip_idx on solve_stat(ip);
create index solve_log_date_idx on solve_stat(log_date);

CREATE OR REPLACE FUNCTION insert_solve_stat(asrc_name text,aip text,aroot_name text,alog_date date,asolve_count integer) RETURNS VOID AS $$
BEGIN
  UPDATE solve_stat SET solve_count=solve_count+asolve_count WHERE src_name = asrc_name AND ip=aip AND root_name=aroot_name AND log_date=alog_date;
  IF found THEN
	RETURN;
  END IF;
	
  INSERT INTO solve_stat(ip,src_name,root_name,log_date,solve_count) values(aip,asrc_name,aroot_name,alog_date,asolve_count);
END;
$$ LANGUAGE plpgsql;



GRANT ALL ON solve_stat to "www-data";
GRANT EXECUTE ON FUNCTION insert_solve_stat(asrc_name text,aip text,aroot_name text,alog_date text,asolve_count integer) to "www-data";
GRANT ALL ON SEQUENCE solve_stat_id_seq to "www-data";
