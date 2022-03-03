library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;
use ieee.std_logic_textio.all;
use ieee.numeric_std.all;
use std.textio.all;

entity testbench is
end entity testbench;

architecture behavioral of testbench is

   signal smclk   : std_logic := '1';
   signal smclkd2 : std_logic := '1';
   signal ledf  : std_logic;
   signal p3m   : std_logic_vector(7 downto 0) := (others=>'Z');   
   signal afbus : std_logic_vector(11 downto 0) :=(others =>'Z');
   signal xbus  : std_logic_vector(45 downto 0) :=(others =>'Z');
   signal rdbus : std_logic_vector(7 downto 0) :=(others =>'Z');
   signal ldbus : std_logic_vector(7 downto 0) :=(others =>'Z');
   signal ispi_clk : std_logic:='1';
   signal ispi_cs  : std_logic:='1';
   signal ispi_di  : std_logic:='Z';    
   signal ispi_do  : std_logic:='1';
   
begin
   --Unit under test
   uut: entity work.fpga
      port map(
         SMCLK    => smclk,
         ACLK     => '0',
         FCLK     => '0',
         LEDF     => ledf,
         SPI_CLK  => ispi_clk,
         SPI_CS   => '1',
         SPI_FPGA_CS => ispi_cs,
         SPI_DI   => ispi_di,
         SPI_DO   => ispi_do,
         KIN      => open,
         KOUT     => (others => 'Z'),
         LE       => open,
         LRW      => open,
         LRS      => open,
         LD       => ldbus,
         RA       => open,
         RD       => rdbus,
         RDQM     => open,
         RCS      => open,
         RRAS     => open,
         RCAS     => open,
         RWE      => open,
         RCKE     => open,
         RCLK     => open,
         P3M      => p3m,
         AFBUS    => afbus,
   	   X        => xbus
   );

   clkgen : process
   begin
      smclk<= '1';
      wait for 67.5 ns;
      smclk <= '0';
      wait for 67.5 ns;
   end process;

   spiclkgen: process(smclk)
   begin
      if (smclk'event) and (smclk='1')then
         smclkd2 <= not smclkd2;
         if (smclkd2 = '1') then
            ispi_clk <= not ispi_clk;
         end if;
      end if;
   end process;


   write:process
   begin
		p3m(0) <= '0';
      wait;
   end process;

end architecture behavioral;
