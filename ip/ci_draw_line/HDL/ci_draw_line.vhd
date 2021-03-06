-- ---------------------------------------------------------------------------
-- Custom Instruction: Draw Line
--
-- This custom instruction draws a line with a solid colour.
--
-- Author: Stephen Just & Stefan Martynkiw
-- ---------------------------------------------------------------------------

LIBRARY ieee;
USE ieee.numeric_std.all;
USE ieee.std_logic_1164.all;

ENTITY ci_draw_line IS
	PORT (
		clk              : in     std_logic; -- Only keeping QSYS happy - use ncs_ci_clk
		reset            : in     std_logic; -- Only keeping QSYS happy - use ncs_ci_reset

		-- Custom instruction ports
		ncs_ci_clk       : in     std_logic;
		ncs_ci_clk_en    : in     std_logic;
		ncs_ci_reset     : in     std_logic;
		ncs_ci_start     : in     std_logic;
		ncs_ci_done      : buffer std_logic;

		-- Memory-mapped slave for config registers
		avs_s0_write     : in     std_logic;
		avs_s0_writedata : in     std_logic_vector(31 downto 0);
		avs_s0_address   : in     std_logic_vector(2 downto 0);

		-- Memory-mapped master for DMA write
		avm_m0_write       : buffer std_logic;
		avm_m0_writedata   : buffer std_logic_vector(7 downto 0); -- Want 1 byte for writing to SRAM
		avm_m0_burstcount  : buffer std_logic_vector(7 downto 0);
		avm_m0_address     : buffer std_logic_vector(31 downto 0);
		avm_m0_waitrequest : in     std_logic
	);
END ENTITY;

ARCHITECTURE Behavioral OF ci_draw_line IS

	TYPE op_state IS (IDLE, RUNNING);

	CONSTANT FRAME_WIDTH : signed(15 downto 0) := to_signed(640, 16);
	CONSTANT FRAME_HEIGHT : signed(15 downto 0) := to_signed(480, 16);

	CONSTANT REG_BUF_ADDR : std_logic_vector(2 downto 0) := B"000";
	CONSTANT REG_X1 : std_logic_vector(2 downto 0) := B"001";
	CONSTANT REG_Y1 : std_logic_vector(2 downto 0) := B"010";
	CONSTANT REG_X2 : std_logic_vector(2 downto 0) := B"011";
	CONSTANT REG_Y2 : std_logic_vector(2 downto 0) := B"100";
	CONSTANT REG_COLOR : std_logic_vector(2 downto 0) := B"101";

	-- Configuration registers
	SIGNAL buf_addr : std_logic_vector(31 downto 0) := (others => '0');
	SIGNAL x1 : std_logic_vector(15 downto 0) := (others => '0');
	SIGNAL y1 : std_logic_vector(15 downto 0) := (others => '0');
	SIGNAL x2 : std_logic_vector(15 downto 0) := (others => '0');
	SIGNAL y2 : std_logic_vector(15 downto 0) := (others => '0');
	SIGNAL color : std_logic_vector(7 downto 0) := (others => '0');

	SIGNAL current_state : op_state := IDLE;
	SIGNAL current_x : std_logic_vector(15 downto 0);
	SIGNAL current_y : std_logic_vector(15 downto 0);


    --For the line algorithm
    signal dx : signed(15 downto 0);
    signal dy : signed(15 downto 0);

    -- Because sx and sy are not assigned in the loop, these should be
    -- signals and not variables.
    signal sx : signed(15 downto 0);
    signal sy : signed(15 downto 0);

    signal p_error : signed(15 downto 0); --Persistent error.


BEGIN



    -- next_x and next_y need to be set as variables within the process block
    -- so that they can be updated at not just the end of a clock cycle.

	-- Do operation
	op : PROCESS(ncs_ci_clk)
		VARIABLE next_x : std_logic_vector(15 downto 0);
		VARIABLE next_y : std_logic_vector(15 downto 0);


        --Bresenham's Line Algorithm adapted from slide 6
        -- http://www1.cs.columbia.edu/~sedwards/classes/2012/4840/lines.pdf

        variable err : signed(15 downto 0);
        variable e2  : signed(15 downto 0);
	BEGIN



		IF rising_edge(ncs_ci_clk) THEN
            IF reset = '1' OR ncs_ci_reset = '1' THEN
                current_state <= IDLE;
		    ELSIF current_state = IDLE THEN -- Don't do anything. 
				ncs_ci_done <= '0';
				avm_m0_write <= '0';
				IF ncs_ci_start = '1' THEN
					current_state <= RUNNING;
					current_x <= x1;
					current_y <= y1;

                    -- Need to set up the dx, dy which is constant after
                    -- updating the configuration registers.

                    -- Set delta_x and delta_y for the whole line draw. It shouldn't matter if 
                    -- we are running or not, since this is invariant for the whole draw.
                    dx <= abs(signed(x2) - signed(x1)); --abs(x1 - x0)
                    dy <= abs(signed(y2) - signed(y1)); --abs(y1 - y0)
                    if signed(x1) < signed(x2) then 
                        sx <= to_signed(1, 16); 
                    else 
                        sx <= to_signed(-1, 16); 
                    end if;

                    if signed(y1) < signed(y2) then 
                        sy <= to_signed(1, 16); 
                    else 
                        sy <= to_signed(-1, 16); 
                    end if;

                    --p_error <= dx - dy;
                      p_error <= abs(signed(x2) - signed(x1)) - abs(signed(y2) - signed(y1));
				ELSE
					current_state <= IDLE;
				END IF;
			ELSIF current_state = RUNNING THEN  -- This is where we are drawing
                -- These signals are only assigned once we've finished the process.
				avm_m0_burstcount <= x"01";
				avm_m0_writedata <= color; 
				avm_m0_write <= '1';
				next_x := current_x;
				next_y := current_y;




				-- Wait until write succeeds to advance to next pixel
				IF avm_m0_waitrequest = '0' THEN
                    err := p_error;
					IF current_y = y2 and current_x = x2 THEN
						-- Write completed
                        -- We've reached the end of the line at x2,y2
						current_state <= IDLE;
						avm_m0_write <= '0';
						ncs_ci_done <= '1'; --Assert frame drawing done.
                    else
                        -- We haven't finished drawing the line yet.
                        -- Bresenham's Line Algorithm goes. here.



                        e2 := err sll 2; --2 is an int, not signed. wtf.

                        if e2 > (to_signed(-1, 16) * dy) then 
                            err := err - dy;
                            next_x := std_logic_vector(signed(current_x) + signed(sx));
                        end if;

                        if e2 < dx then 
                            err := err + dx;
                            next_y := std_logic_vector(signed(current_y) + signed(sy));
                        end if;
                        
                        p_error <= err;

                    end if;
				END IF;




				avm_m0_address <= std_logic_vector(unsigned(buf_addr) + unsigned(next_x) + (unsigned(FRAME_WIDTH) * unsigned(next_y)));
				current_x <= next_x;
				current_y <= next_y;
			END IF;
		END IF;
	END PROCESS op;

	-- Update configuration registers
	-- In the case of drawing a rectangle, we can do some bounds-checking on
	-- the inputs now. In other cases (such as drawing a line) we might want
	-- to allow coordinates outside of the frame window, which would be clipped
	-- during the memory-writing stage of command execution.

    -- Registers: framebuffer00, x1, y1, x2, y2, color


	update_cfg : PROCESS(ncs_ci_clk)
	BEGIN
		IF rising_edge(ncs_ci_clk) THEN
			IF ncs_ci_reset = '1' THEN
				buf_addr <= (others => '0');
				x1 <= (others => '0');
				y1 <= (others => '0');
				x2 <= (others => '0');
				y2 <= (others => '0');
				color <= (others => '0');
			ELSIF avs_s0_write = '1' THEN
				CASE avs_s0_address IS
					WHEN REG_BUF_ADDR =>
						buf_addr <= avs_s0_writedata; -- Set the address of the framebuffer to write to. (Address of 0,0)
					WHEN REG_X1 =>
						IF signed(avs_s0_writedata(15 downto 0)) < to_signed(0, 16) THEN
							x1 <= (others => '0');
						ELSIF signed(avs_s0_writedata(15 downto 0)) >= FRAME_WIDTH THEN
							x1 <= std_logic_vector(FRAME_WIDTH - to_signed(1, 16));
						ELSE
							x1 <= avs_s0_writedata(15 downto 0);
						END IF;
					WHEN REG_Y1 =>
						IF signed(avs_s0_writedata(15 downto 0)) < to_signed(0, 16) THEN
							y1 <= (others => '0');
						ELSIF signed(avs_s0_writedata(15 downto 0)) >= FRAME_HEIGHT THEN
							y1 <= std_logic_vector(FRAME_HEIGHT - to_signed(1, 16));
						ELSE
							y1 <= avs_s0_writedata(15 downto 0);
						END IF;
					WHEN REG_X2 =>
						IF signed(avs_s0_writedata(15 downto 0)) < to_signed(0, 16) THEN
							x2 <= (others => '0');
						ELSIF signed(avs_s0_writedata(15 downto 0)) >= FRAME_WIDTH THEN
							x2 <= std_logic_vector(FRAME_WIDTH - to_signed(1, 16));
						ELSE
							x2 <= avs_s0_writedata(15 downto 0);
						END IF;
					WHEN REG_Y2 =>
						IF signed(avs_s0_writedata(15 downto 0)) < to_signed(0, 16) THEN
							y2 <= (others => '0');
						ELSIF signed(avs_s0_writedata(15 downto 0)) >= FRAME_HEIGHT THEN
							y2 <= std_logic_vector(FRAME_HEIGHT - to_signed(1, 16));
						ELSE
							y2 <= avs_s0_writedata(15 downto 0);
						END IF;
					WHEN REG_COLOR =>
						color <= avs_s0_writedata(7 downto 0);
					WHEN OTHERS =>
						-- Do nothing
				END CASE;
			END IF;
		END IF;
	END PROCESS update_cfg;

END ARCHITECTURE;
