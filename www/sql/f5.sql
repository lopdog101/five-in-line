
CREATE TABLE solve_stat(
    id          serial PRIMARY KEY,
    src_name    text,
	ip          text,
    log_date    date,
	solve_count integer NOT NULL default 0,
	unique(src_name,ip,log_date)
);

create index solve_stat_src_name_idx on solve_stat(src_name);
create index solve_stat_ip_idx on solve_stat(ip);
create index solve_log_date_idx on solve_stat(log_date);

GRANT ALL ON solve_stat to "www-data";
