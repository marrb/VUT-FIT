-- uart.vhd: UART controller - receiving part
-- Author(s): 
--
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;
use IEEE.numeric_std.ALL;

-------------------------------------------------
entity UART_RX is
port(	
   CLK: 	    	in std_logic;
	RST: 	    	in std_logic;
	DIN: 	    	in std_logic;
	DOUT: 	   out std_logic_vector(7 downto 0);
	DOUT_VLD: 	out std_logic
);
end UART_RX;  

-------------------------------------------------
architecture behavioral of UART_RX is
signal cnt: std_logic_vector(4 downto 0);
signal cnt_2: integer := 0;
signal cnt_en: std_logic;
signal rec_en: std_logic;
signal data_vld: std_logic;

begin
	I_UART: entity work.UART_FSM(behavioral)
    port map (
        CLK 	    => CLK,
        RST 	    => RST,
        D_IN 	    => DIN,
        CNT 	    => cnt,
		  CNT_2  	 => cnt_2,
		  CNT_EN		 => cnt_en,
		  REC_EN     => rec_en,
		  DATA_VLD	 => data_vld 
    );
	 process(CLK) begin
		if(falling_edge(CLK)) then
			if(cnt_en = '1') then
				cnt <= cnt + 1;
				if(rec_en = '1') then
					if((to_integer(unsigned(cnt))) >= 15) then
						DOUT(cnt_2) <= DIN;
						cnt_2 <= cnt_2 + 1;
						cnt <= (others => '0');
					end if;
				end if;
			else
				cnt <= (others => '0');
				cnt_2 <= 0;
			end if;
			if(data_vld = '1') then
				DOUT_VLD <= '1';
			else
				DOUT_VLD <= '0';
			end if;
		end if;
	 end process;
end behavioral;
