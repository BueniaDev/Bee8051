#include <Bee8051/bee8051.h>
#include <fstream>
#include <SDL2/SDL.h>
using namespace bee8051;
using namespace std;

class Sim8051 : public Bee8051Interface
{
    public:
	Sim8051()
	{
	    core.setInterface(this);
	}

	~Sim8051()
	{

	}

	bool init(string filename)
	{
	    main_rom.fill(0);
	    if (!process_intel_hex_file(filename))
	    {
		return false;
	    }

	    if (SDL_Init(SDL_INIT_VIDEO) < 0)
	    {
		return sdl_error("SDL2 could not be initialized!");
	    }

	    window = SDL_CreateWindow("Sim8051", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_SHOWN);

	    if (window == NULL)
	    {
		return sdl_error("Window could not be created!");
	    }

	    core.init();

	    for (int i = 0; i < 5; i++)
	    {
		core.debugoutput();
		core.runinstruction();
	    }

	    return true;
	}

	void shutdown()
	{
	    core.shutdown();

	    if (window != NULL)
	    {
		SDL_DestroyWindow(window);
		window = NULL;
	    }

	    SDL_Quit();
	}

	void run()
	{
	    bool quit = false;
	    SDL_Event event;

	    while (!quit)
	    {
		while (SDL_PollEvent(&event))
		{
		    switch (event.type)
		    {
			case SDL_QUIT: quit = true; break;
		    }
		}

		runcore();
	    }
	}

	uint8_t readROM(uint16_t addr)
	{
	    return main_rom.at(addr);
	}

	void portOut(int port, uint8_t data)
	{
	    (void)port;
	    (void)data;
	    return;
	}

    private:
	bool process_intel_hex_file(string filename)
	{
	    ifstream file(filename);

	    if (!file.is_open())
	    {
		return false;
	    }

	    string line;

	    while (getline(file, line))
	    {
		if (line.at(0) != ':')
		{
		    cout << "Invalid HEX file" << endl;
		    return false;
		}

		uint32_t byte_count = from_hex_str(line.substr(1, 2));
		uint32_t mem_addr = from_hex_str(line.substr(3, 4));
		uint8_t record_type = from_hex_str(line.substr(7, 2));

		switch (record_type)
		{
		    case 0x00:
		    {
			for (size_t index = 0; index < byte_count; index++)
			{
			    uint32_t byte_offs = (9 + (index * 2));
			    uint8_t data_byte = from_hex_str(line.substr(byte_offs, 2));
			    main_rom.at(mem_addr + index) = data_byte;
			}
		    }
		    break;
		    case 0x01: break; // End of file
		    default:
		    {
			cout << "Unrecognized record type of " << hex << int(record_type) << endl;
			return false;
		    }
		    break;
		}
	    }

	    return true;
	}

	uint32_t from_hex_str(string hex_str)
	{
	    return strtoul(hex_str.c_str(), NULL, 16);
	}

	bool sdl_error(string msg)
	{
	    cout << msg << " SDL_Error: " << SDL_GetError() << endl;
	    return false;
	}

	void runcore()
	{
	    uint32_t cycle_count = (from_mhz(12) / 60);
	    uint32_t cycles = 0;

	    while (cycles < cycle_count)
	    {
		// core.debugoutput();
		cycles += core.runinstruction();
	    }
	}

	Bee8051 core;

	SDL_Window *window = NULL;
	SDL_Renderer *render = NULL;

	uint32_t from_mhz(uint32_t mhz)
	{
	    return (mhz * 1e6);
	}

	array<uint8_t, 0x1000> main_rom;
};

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
	cout << "Usage: sim8051 <Intel HEX file>" << endl;
	return 1;
    }

    Sim8051 core;

    if (!core.init(argv[1]))
    {
	return 1;
    }

    core.run();
    core.shutdown();
    return 0;
}