LIBRARY ieee;
USE ieee.std_logic_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
USE ieee.numeric_std.ALL;
 
ENTITY counter_tb IS
END counter_tb;
 
ARCHITECTURE behavior OF counter_tb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT counter
	 Generic (
			CLK_FREQ : positive := 100000;
			OUT_FREQ : positive := 1000);	
    PORT(
         CLK : IN  std_logic;
         RESET : IN  std_logic;
         EN : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal CLK : std_logic := '0';
   signal RESET : std_logic := '0';

 	--Outputs
   signal EN : std_logic;

   -- Clock period definitions
   constant CLK_period : time := 1 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
	-- vas ukol !!!!!!!!! vcetne nastaveni generickych parametru
	i_counter : entity work.counter(Behavioral)
	generic map(CLK_FREQ => 10000,
					OUT_FREQ => 1000)
	port map(CLK => CLK,
				RESET => RESET,
				EN => EN);
	
   -- Clock process definitions
   CLK_process :process
   begin
		CLK <= '0';
		wait for CLK_period/2;
		CLK <= '1';
		wait for CLK_period/2;
   end process;
 
	-- Stimulus process
   stim_proc: process
   begin
      -- vas ukol !!!!!!!!! nastavte spravne reset, spuste pocitani
		RESET <= '0';
		
		wait for 10ns;
		assert (EN'stable(9.5ns) = true) report "Check Passed, EN = 0 for 9.5ns!" severity note;
		assert (EN'stable(10ns)) report "Check Passed, EN = changed from 0 to 1 for 0.5ns!" severity note;
		assert (EN'stable(10ns) = false) report "Check Failed!" severity note;

   end process;
END;