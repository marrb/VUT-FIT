#!/bin/sh

export POSIXLY_CORRECT=yes
export LC_NUMERIC=en_US.UTF-8

#Ukaze pomoc a ukonci program--------------------------------------------------------------------
show_help()
{
echo "Usage: tradelog [-h|--help]"
echo "       tradelog [FILTER] [COMMAND] [LOG [LOG2 [...]]"
echo ""
echo "Filters:
      -a, --DATETIME	only entries after this date [YYYY-MM-DD HH:MM:SS]
      -b, --DATETIME	only entries before this date [YYYY-MM-DD HH:MM:SS]
      -t, --TICKER	only entries with chosen ticker are considered
      -w, --WIDTH	set width of graph (lenghth of the longest row), [1 - inf]
      -h/--help,	show this help and exit"
echo ""
echo "Commands:
      list-tick,	list tickers
      profit,		list total profit from closed positions
      pos,		list values of currently held positions, sorted by value
      last-price,	list last known price for every ticker
      hist-ord,		histogram report of the number of transactions by ticker
      graph-pos,	show graph of values of held positios by ticker"
echo""
rem_log
exit
}
#------------------------------------------------------------------------------------------------

#V pripade pozitia zazipovanych suborov, ktore sa rozbalia ich po pouziti vymaze-----------------
rem_log(){
for i in "${REM[@]}"
do
        rm $i
done
}
#------------------------------------------------------------------------------------------------

#Spracovava subor ked nie je urceny prikaz-------------------------------------------------------
process_command(){
for j in "${LOG[@]}"
do
        if $TICKER_FLAG
        then
        for i in "${TICKER[@]}"
        do
                awk -F';' -v T="$i" -v DA="$DATE_A" -v DB="$DATE_B" '{
                if (DB != 0 && $2==T && ($1 < DB)) print $0;
                if (DA != 0 && $2==T && ($1 > DA)) print $0;               
                if (DA == 0 && DB == 0 && $2==T) print $0;
                }' /$PWD/$j
        done

        else
                awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" '{
                if (DB != 0 || DA != 0){
                        if (DB != 0 && $1 < DB) print $0;
                        if (DA != 0 && $1 > DA) print $0;
                }
                else print $0;
                }' /$PWD/$j
        fi
done
}
#------------------------------------------------------------------------------------------------

#Vypocita profit---------------------------------------------------------------------------------
calc_profit(){
RESULT=0.00
TMP=0.00
for j in "${LOG[@]}"
do	
	#Prejde vsetky tickery, vypocita profit a na konci posle na vystup------------
	if $TICKER_FLAG
	then
	for i in "${TICKER[@]}"
	do
		TMP=$(awk -F';' -v T="$i" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
		if (DB != 0 && $2==T && ($1 < DB)){
			if ($3=="sell") P+=$4*$6;
			else P-=$4*$6;
		}
		if (DA != 0 && $2==T && ($1 > DA)){
			if ($3 == "sell") P+=$4*$6;
			else P-=$4*$6;
		}
		if (DA == 0 && DB == 0 && $2==T){
			if ($3=="sell") P+=$4*$6;
                        else P-=$4*$6;
		}
		
		} END {
		printf("%.2f\n", P)
		}' /$PWD/$j)
		RESULT=$(echo $RESULT+$TMP | bc)
	done
	#Pripad ked nie je pouzity filter '-t'---------------------------------------
	else
               	TMP=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
		if (DB != 0 || DA != 0){
			if (DB != 0 && $1 < DB){
				if ($3=="sell") P+=$4*$6;
				else P-=$4*$6;
			}
			if (DA != 0 && $1 > DA){
                        	if ($3=="sell") P+=$4*$6;
                        	else P-=$4*$6;
                	}
		}
		else{
			if ($3=="sell") P+=$4*$6;
			else P-=$4*$6;
		}
                        
		} END {
		printf("%.2f\n", P)
		}' /$PWD/$j)
               	RESULT=$(echo $RESULT+$TMP | bc)
	fi
done
echo "$RESULT"
}
#------------------------------------------------------------------------------------------------

#Vypise vsetky tickery bez opakovania------------------------------------------------------------
list_tickers(){
for i in "${LOG[@]}"
do
	RECORDS+=$(awk -F';' '{ !seen[$2]++ } {printf "%s\n",$2  }' $PWD/$i)
done

echo "$RECORDS" | awk '!seen[$0]++ { print }' | sort
}
#------------------------------------------------------------------------------------------------

#Vypise aktualne drzanie pozicie-----------------------------------------------------------------
list_pos()
{
RESULT=0.00
TMP=0.00
PRICE=0.00
for i in "${TICKER[@]}"
do
	RESULT=0.00
	for j in "${LOG[@]}"
	do
		TMP=$(awk -F';' -v T="$i" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
                if (DB != 0 && $2l==T && ($1 < DB)){
                        if ($3=="sell") P-=$6;
                        else P+=$6;
                }
                if (DA != 0 && $2==T && ($1 > DA)){
                        if ($3 == "sell") P-=$6;
                        else P+=$6;
                }
		if (DA == 0 && DB == 0 && $2==T){
                        if ($3=="sell") P-=$6;
                        else P+=$6;
                }
                
                } END {
                printf("%.2f\n", P)
                }' /$PWD/$j)
		
		#Najde poslednu cenu aktualne spracovavaneho tickeru------------------
		PRICE=$(awk -F';' -v T="$i" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
			if ($2 == T && $1 > DATE) P=$4;
		} END {
		printf("%.2f\n", P)	
		}' /$PWD/$j)
		
		TMP=$(echo $TMP*$PRICE | bc)
                RESULT=$(echo $RESULT+$TMP | bc)

	done
	printf  "%-9s : %11s\n" $i $RESULT
done

#Pripad ked nie je nastevent '-t' filter
if ! $TICKER_FLAG
then	
	#Prejde vsetky tickery a vypise
	while IFS= read -r line;
	do	
		RESULT=0.00
		for j in "${LOG[@]}"
       		do
		
			TMP=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 -v T="$line" '{
			if (DB != 0 && $2==T && ($1 < DB)){
                        	if ($3=="sell") P-=$6;
              	          	else P+=$6;
                	}
	                if (DA != 0 && $2==T && ($1 > DA)){
        	                if ($3 == "sell") P-=$6;
                	        else P+=$6;
	                }
        	        if (DA == 0 && DB == 0 && $2==T){
                	        if ($3=="sell") P-=$6;
	                        else P+=$6;
        	        }
                
	                } END {
        	        printf("%.2f\n", P)
                	}' /$PWD/$j)

		PRICE=$(awk -F';' -v T="$line" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
                        if ($2 == T && $1 > DATE) P=$4;
                } END {
                printf("%.2f\n", P)
                }' /$PWD/$j)
		
		TMP=$(echo $TMP*$PRICE | bc)
		RESULT=$(echo $RESULT+$TMP | bc)

		done
		printf  "%-9s : %11s\n" $line $RESULT
	done <<< "$TICKERS"
fi
}
#-----------------------------------------------------------------------------------------------

#Vypise poslednu cenu kazdeho tickeru-----------------------------------------------------------
show_lprice(){
LPRICE=0.00
for i in ${LOG[@]}
do	
	echo "LOG: $i"
	while IFS= read -r line;
	do
		LPRICE=$(awk -F';' -v T="$line" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
        	if ($2 == T && $1 > DATE) P=$4;
        	} END {
        	printf("%.2f\n", P)
        	}' /$PWD/$i)
		
		printf  "%-9s : %7s\n" $line $LPRICE
	done <<< "$TICKERS"
done
}
#-----------------------------------------------------------------------------------------------

#Vypise pocet transakcii------------------------------------------------------------------------
show_h_ord(){
N=0
TMP=0
TMP_MAX=0	
MAX=0
DIV=0.000

#Pripad ked je aktivny filter '-t'
if $TICKER_FLAG
then

#Ak je nastaveny '-w' filter najde najvatsiu hodnotu a podla nej vypocita ostatne	
if (( $WIDTH != 0 ))
then	
	for i in "${TICKER[@]}"
	do	
		TMP=0
		for j in "${LOG[@]}"
		do
			TMP_MAX=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v T="$i" -v P=0 '{
		                if (DB != 0 && $2==T && ($1 < DB)) P+=1;
		                if (DA != 0 && $2==T && ($1 > DA)) P+=1;
		                if (DA == 0 && DB == 0 && $2==T) P+=1;
		                } END {
		                printf("%d\n", P)
			}' /$PWD/$j)
			TMP=$(echo $TMP_MAX+$TMP | bc)
		done
		if (( $MAX < $TMP ))
		then
			MAX=$TMP
		fi
	done
DIV=$(echo "scale=3; $WIDTH / $MAX" | bc)     

fi

#Prejde vsetky tickeri
for i in "${TICKER[@]}"
do
	N=0
        for j in "${LOG[@]}"
        do	
                TMP=$(awk -F';' -v T="$i" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0 '{
                if (DB != 0 && $2==T && ($1 < DB)) P+=1;
                if (DA != 0 && $2==T && ($1 > DA)) P+=1;
                if (DA == 0 && DB == 0 && $2==T) P+=1;
                } END {
                printf("%d\n", P)
                }' /$PWD/$j)
		N=$(echo $TMP+$N | bc)
        done
	if (( $WIDTH != 0 ))
	then
	#Ak je nastaveny '-w' na vystup dava podla jeho hodnoty
	printf  "%-9s : %-1s\n" $i $(awk -v i=$(echo "scale=3; $N*$DIV" | bc) -v num=$N 'BEGIN {
                if (num != 0){
                OFS="#"; $i="#"; print
                }}')
	else
	#Ak nie tak dava podla poctu transakcii
	printf  "%-9s : %-1s\n" $i $(awk -v i=$N 'BEGIN {
                if (i != 0){
                OFS="#"; $i="#"; print
                }}')	
	fi
done
else	
	#Ak je nastaveny '-w' a nie je nastaveny '-t' tak prejde vsetky tickeri a najde maximum
	if (( $WIDTH != 0 ))
	then
		while IFS= read -r line;
		do
			TMP=0
                	for j in "${LOG[@]}"
                	do
	                        TMP_MAX=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v T="$line" -v P=0 '{
        	                if (DB != 0 || DA != 0){
                                	if (DB != 0 && $1 < DB && $2==T) P+=1;
                                	if (DA != 0 && $1 > DA && $2==T) P+=1;
                        	}
                      		else{
                        	        if ($2==T) P+=1;
                        	}
                                } END {
	                        printf("%d\n", P)
        	                }' /$PWD/$j)
                	        TMP=$(echo $TMP_MAX+$TMP | bc)
                	done
                if (( $MAX < $TMP ))
                then
                        MAX=$TMP
                fi
		done <<< "$TICKERS"
		DIV=$(echo "scale=3; $WIDTH / $MAX" | bc)
	fi
	
	#Prejde vsetky tickeri a posiela na vystup
	while IFS= read -r line;
 	do
		N=0
		for j in "${LOG[@]}"
        	do
                	TMP=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v P=0 -v T="$line" '{
                	if (DB != 0 || DA != 0){
                	        if (DB != 0 && $1 < DB && $2==T) P+=1;
                        	if (DA != 0 && $1 > DA && $2==T) P+=1;
	                }
			else{
				if ($2==T) P+=1;
			}
        	                
                	} END {
	                printf("%d\n", P)
        	        }' /$PWD/$j)
			N=$(echo $TMP+$N | bc)
		done
	if (( $WIDTH != 0 ))
        then
        	printf  "%-9s : %-1s\n" $line $(awk -v i=$(echo "scale=3; $N*$DIV" | bc) 'BEGIN { OFS="#"; $i="#"; print}')
        else    
        	printf  "%-9s : %-1s\n" $line $(awk -v i=$N 'BEGIN { OFS="#"; $i="#"; print }')
        fi
	done <<< "$TICKERS"
fi
}
#------------------------------------------------------------------------------------------------

#Vypisuje graf hodnot---------------------------------------------------------------------------
show_graph(){

RESULT=0.00
TMP=0.00
PRICE=0.00
MAX=0.00
DIV=0.00

if $TICKER_FLAG
then
#Ak je nastaveny '-w' parameter tak najde maximum a podla neho pocita ostatne hodnoty
if (( $WIDTH != 0 ))
then
        for i in "${TICKER[@]}"
        do
                TMP=0
		RESULT=0
                for j in "${LOG[@]}"
                do

			TMP=$(awk -F';' -v T="$i" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
                	if (DB != 0 && $2l==T && ($1 < DB)){
                        	if ($3=="sell") P-=$6;
	                        else P+=$6;
        	        }
                	if (DA != 0 && $2==T && ($1 > DA)){
	                        if ($3 == "sell") P-=$6;
        	                else P+=$6;
                	}
	                if (DA == 0 && DB == 0 && $2==T){
        	                if ($3=="sell") P-=$6;
                	        else P+=$6;
	                }
        	        
	                } END {
        	        printf("%.2f\n", P)
                	}' /$PWD/$j)

			PRICE=$(awk -F';' -v T="$i" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
                        if ($2 == T && $1 > DATE) P=$4;
	                } END {
        	        printf("%.2f\n", P)     
                	}' /$PWD/$j)

			TMP=$(echo $TMP*$PRICE | bc)
			RESULT=$(echo $RESULT+$TMP | bc)					
		done
		MAX=$(awk -v n1="$RESULT" -v n2="$MAX" -v res=0.00 'BEGIN {
			if (n1 < 0 && n2 < 0 && n1 < n2) res=n1;
			else if (n1 > 0 && n2 > 0 && n1 > n2) res=n1;
			else if (n1 < 0 && n2 > 0 && n1 < (-n2)) res=n1;
			else if (n1 > 0 && n2 < 0 && n1 > (-n2)) res=n1;
			else if (n2==0) res=n1;
			else res=n2;
			printf("%.2f\n", res)
			}')
        done

	DIV=$(echo "scale=2; $MAX / $WIDTH" | bc)
	DIV=$(awk -v res=$DIV 'BEGIN {
        if (res < 0) res=(-res);
        else res=res;
        printf("%.2f\n", res)
        }')
else
	#V pripade ze nie je '-w' nastaveny tak sa hodnoty delia cislom 1000
	DIV=1000
fi

for j in "${TICKER[@]}"
do
        RESULT=0.00
        for i in "${LOG[@]}"
        do
                TMP=$(awk -F';' -v T="$j" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
                if (DB != 0 && $2l==T && ($1 < DB)){
                        if ($3=="sell") P-=$6;
                        else P+=$6;
                }
                if (DA != 0 && $2==T && ($1 > DA)){
                        if ($3 == "sell") P-=$6;
                        else P+=$6;
                }
                if (DA == 0 && DB == 0 && $2==T){
                        if ($3=="sell") P-=$6;
                        else P+=$6;
                }
                
                } END {
                printf("%.2f\n", P)
                }' /$PWD/$i)

                PRICE=$(awk -F';' -v T="$j" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
                        if ($2 == T && $1 > DATE) P=$4;
                } END {
                printf("%.2f\n", P)     
                }' /$PWD/$i)

                TMP=$(echo $TMP*$PRICE | bc)
                RESULT=$(echo $RESULT+$TMP | bc)
        done
	
	#Ak vysledok vysiel zaporne  nastavi sa 'NEG' na 1
	NEG=$(awk -v res=$RESULT -v N=0 'BEGIN {
        if (res < 0) N=1;
        else N=0;
        printf("%d\n", N)
        }')
	
	#Dalej ak vysiel zaporne zmeni sa na znamienko aby nenastala chyba pri deleni
	RESULT=$(awk -v res=$RESULT 'BEGIN {
	if (res < 0) res=(-res);
	else res=res;
	printf("%.2f\n", res)
	}')
	
	#Vypocita pocet znakov ktory sa vypise na vystup
	NUM=$(awk -v res=$RESULT -v div=$DIV -v N=0 'BEGIN {
	N=res/div
	printf("%d\n", N)
	}')
	
	#Ak vyslo 'NEG' 0 tak sa vytlaci znak "#" inak "!"
	if [ $NEG =  0 ]
	then
		printf  "%-9s : %-1s\n" $j $(awk -v i=$(echo "scale=0; $RESULT/$DIV" | bc) -v N=$NUM 'BEGIN {
                if (N != 0){
                OFS="#"; $i="#"; print
                }}')
	else
		printf  "%-9s : %-1s\n" $j $(awk -v i=$(echo "scale=0; $RESULT/$DIV" | bc) -v N=$NUM 'BEGIN {
		if (N != 0){
		OFS="!"; $i="!"; print
		}}')
	fi  
done

else

#Prakticky to iste zo zmenou ze nie je nastaveny parameter '-t' a prechadzaju sa vsetky tickeri
if (( $WIDTH != 0 ))
then
        while IFS= read -r line;
        do
                TMP=0
                RESULT=0
                for j in "${LOG[@]}"
                do
                        TMP=$(awk -F';' -v T="$line" -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 '{
                        if (DB != 0 && $2l==T && ($1 < DB)){
                                if ($3=="sell") P-=$6;
                                else P+=$6;
                        }
                        if (DA != 0 && $2==T && ($1 > DA)){
                                if ($3 == "sell") P-=$6;
                                else P+=$6;
                        }
                        if (DA == 0 && DB == 0 && $2==T){
                                if ($3=="sell") P-=$6;
                                else P+=$6;
                        }
                        
                        } END {
                        printf("%.2f\n", P)
                        }' /$PWD/$j)

                        PRICE=$(awk -F';' -v T="$line" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
                        if ($2 == T && $1 > DATE) P=$4;
                        } END {
                        printf("%.2f\n", P)     
                        }' /$PWD/$j)

                        TMP=$(echo $TMP*$PRICE | bc)
                        RESULT=$(echo $RESULT+$TMP | bc)                                        
                done
                MAX=$(awk -v n1="$RESULT" -v n2="$MAX" -v res=0.00 'BEGIN {
                        if (n1 < 0 && n2 < 0 && n1 < n2) res=n1;
                        else if (n1 > 0 && n2 > 0 && n1 > n2) res=n1;
                        else if (n1 < 0 && n2 > 0 && n1 < (-n2)) res=n1;
                        else if (n1 > 0 && n2 < 0 && n1 > (-n2)) res=n1;
                        else if (n2==0) res=n1;
                        else res=n2;
                        printf("%.2f\n", res)
                        }')
        done <<< "$TICKERS"

        DIV=$(echo "scale=2; $MAX / $WIDTH" | bc)
        DIV=$(awk -v res=$DIV 'BEGIN {
        if (res < 0) res=(-res);
        else res=res;
        printf("%.2f\n", res)
        }')
else
        DIV=1000
fi

        while IFS= read -r line;
        do
		
                RESULT=0.00
                for j in "${LOG[@]}"
                do

                        TMP=$(awk -F';' -v DA="$DATE_A" -v DB="$DATE_B" -v P=0.00 -v T="$line" '{
                        if (DB != 0 && $2==T && ($1 < DB)){
                                if ($3=="sell") P-=$6;
                                else P+=$6;
                        }
                        if (DA != 0 && $2==T && ($1 > DA)){
                                if ($3 == "sell") P-=$6;
                                else P+=$6;
                        }
                        if (DA == 0 && DB == 0 && $2==T){
                                if ($3=="sell") P-=$6;
                                else P+=$6;
                        }

                        } END {
                        printf("%.2f\n", P)
                        }' /$PWD/$j)

                PRICE=$(awk -F';' -v T="$line" -v P=0.00 -v DATE="0000-00-00 00:00:00" '{
                        if ($2 == T && $1 > DATE) P=$4;
                } END {
                printf("%.2f\n", P)
                }' /$PWD/$j)

                TMP=$(echo $TMP*$PRICE | bc)
                RESULT=$(echo $RESULT+$TMP | bc)
 
                done

		NEG=$(awk -v res=$RESULT -v N=0 'BEGIN {
	        if (res < 0) N=1;
	        else N=0;
        	printf("%d\n", N)
	        }')

        	RESULT=$(awk -v res=$RESULT 'BEGIN {
	        if (res < 0) res=(-res);
        	else res=res;
	        printf("%.2f\n", res)
        	}')
	
	        NUM=$(awk -v res=$RESULT -v div=$DIV -v N=0 'BEGIN {
        	N=res/div
	        printf("%d\n", N)
        	}')
        
	        if [ $NEG =  0 ]
	        then
                printf  "%-9s : %-1s\n" $line $(awk -v i=$(echo "scale=0; $RESULT/$DIV" | bc) -v N=$NUM 'BEGIN {
                if (N != 0){
                OFS="#"; $i="#"; print
                }}')
	        else
                printf  "%-9s : %-1s\n" $line $(awk -v i=$(echo "scale=0; $RESULT/$DIV" | bc) -v N=$NUM 'BEGIN {
                if (N != 0){
                OFS="!"; $i="!"; print
                }}')
        	fi		

        done <<< "$TICKERS"
fi
}
#------------------------------------------------------------------------------------------------

#Premenne-----------
REM=()
LOG=()
TICKER=() 
TICKER_FLAG=false
DATE_FLAG_A=false
DATE_FLAG_B=false
DATE_A=0
DATE_B=0
PROFIT_FLAG=false
LIST_FLAG=false
POS_FLAG=false
L_PRICE_FLAG=false
H_ORD_FLAG=false
GRAPH_FLAG=false
PROFIT=0
WIDTH=0
COMMAND=0
#------------------

#Prejde vsetky parametre na vstupe, nastavi flagy a podla toho vykona urcity prikaz--------------
while (( $# )); do
	case "$1" in
		-a) #Uvazuju sa len zaznamy po tomto datume (bez tohoto datumu) FORMAT: {YYYY-MM-DD HH:MM:SS}
		DATE_FLAG_A=true
		DATE_A=$2
		shift 2
		continue
		;;

		-b) #Uvazuju sa len zaznamy pred tymto datumom (bez tohoto datumu)
		
		DATE_FLAG_B=true
		DATE_B=$2
		shift 2
		continue
		;;

		-t) #TICKER - Uvazuju sa zaznamy odpovedajuce danemu tickeru, pri viacerych sa berie mnozina vsetkych uvedenych
		TICKER+=("$2")
		TICKER_FLAG=true
		shift 2
		continue
		;;

		-w) #WIDTH - u výpisu grafů nastavuje jejich šířku, tedy délku nejdelšího řádku na WIDTH.
		    #Tedy, WIDTH musí být kladné celé číslo. Více výskytů přepínače je chybné spuštění.

		if (( $WIDTH != 0 ))
                then
                echo "ERROR: Multiple '-w' arguments detected!"
                exit
                fi

		WIDTH=$2
	
		if (( $WIDTH < 1 ))
		then
		echo "ERROR: Invalid value of argument '-w'!";
		show_help
		exit
		fi
		
		shift 2
		continue
		;;
		-h|--help) #vypíšou nápovědu s krátkým popisem každého příkazu a přepínače.
			show_help
		;;
		-*) #Neznamy argument
		echo "Error: Unknown argument detected!
		"
		show_help
		;;
		
		list-tick) #list ticker
			LIST_FLAG=true
			COMMAND="$1"
		;;

		profit) #Zobrazi celkovy zisk
			PROFIT_FLAG=true
			COMMAND="$1"
		;;

		pos)
			POS_FLAG=true
			COMMAND="$1"
		;;

		last-price)
			L_PRICE_FLAG=true
			COMMAND="$1"
		;;

		hist-ord)
			H_ORD_FLAG=true
			COMMAND="$1"
		;;

		graph-pos)
			GRAPH_FLAG=true
			COMMAND="$1"
		;;

		*.log)
		LOG+=("$1")
		if [ ! -f "/$PWD/$1" ]
		then
		echo "ERROR: '$1': File does not exist!"
		exit
		elif [ ! -r "/$PWD/$1" ]
		then
		echo "ERROR: '$1': You do not have permission to read this file!"
		fi
		;;	

		*.gz)
		if [ ! -f "/$PWD/$1" ]
                then
                echo "ERROR: '$1': File does not exist!"
                exit
                elif [ ! -r "/$PWD/$1" ]
                then
                echo "ERROR: '$1': You do not have permission to read this file!"
                fi
		gunzip -k $1
		LOG+=("$(echo "$1" | awk 'BEGIN { FS = ".gz" } ; { print $1 }')")
		REM+=("$(echo "$1" | awk 'BEGIN { FS = ".gz" } ; { print $1 }')")
		;;

	esac
	shift
done
#------------------------------------------------------------------------------------------------

TICKER=($(printf "%s\n" "${TICKER[@]}" | sort -u | tr '\n' ' '))

if [ ${#LOG[@]}  -eq 0 ]
then
    echo "ERROR: Missing file on input!"
    exit
fi

for i in "${LOG[@]}"
do
	TICKERS+=$(awk -F';' '{ !seen[$2]++ } {printf "%s\n",$2  }' $PWD/$i)
done

TICKERS=$(echo "$TICKERS" | awk '!seen[$0]++ { print }' | sort)


#Podla flagov vykona 1 z danych prikazov
if $LIST_FLAG
then
list_tickers
fi

if $POS_FLAG
then
list_pos
fi

if $PROFIT_FLAG
then
calc_profit
fi

if $L_PRICE_FLAG
then
show_lprice
fi

if $H_ORD_FLAG
then
show_h_ord
fi

if $GRAPH_FLAG
then
show_graph
fi

if [ "$COMMAND" == 0 ]
then
process_command
fi

rem_log

exit
