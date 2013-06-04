#ifndef env_variablesH
#define env_variablesH

namespace Gomoku
{

inline void print_enviropment_variables_hint()
{
	printf("Enviropment variables\n");
	printf("stored_deep (default: %u)\n",WsPlayer::stored_deep);
	printf("lookup_deep (default: %u)\n",WsPlayer::def_lookup_deep);
	printf("treat_deep  (default: %u)\n",WsPlayer::treat_deep);
	printf("max_treat_check  (default: %u)\n",WsPlayer::max_treat_check);
	printf("max_treat_check_rebuild_tree  (default: %u)\n",WsPlayer::max_treat_check_rebuild_tree);
}

inline void scan_enviropment_variables()
{
	const char* sval=getenv("stored_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::stored_deep=atol(sval);

	sval=getenv("lookup_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::def_lookup_deep=atol(sval);

	sval=getenv("treat_deep");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::treat_deep=atol(sval);

	sval=getenv("max_treat_check");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::max_treat_check=atol(sval);

	sval=getenv("max_treat_check_rebuild_tree");
	if(sval!=0&&(*sval)!=0)
		WsPlayer::max_treat_check_rebuild_tree=atol(sval);
}

inline void print_used_enviropment_variables(ObjectProgress::log_generator& lg)
{
    lg
        <<"stored_deep="<<WsPlayer::stored_deep
        <<" lookup_deep="<<WsPlayer::def_lookup_deep
        <<" treat_deep="<<WsPlayer::treat_deep
        <<" max_treat_check="<<WsPlayer::max_treat_check
        <<" max_treat_check_rebuild_tree="<<WsPlayer::max_treat_check_rebuild_tree;
}

}

#endif
