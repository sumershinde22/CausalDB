# CausalDB - Causal Event Database

A simple causal event database implemented in C with a modern web frontend. This project allows you to store events with causal relationships between them, creating a directed graph of dependencies.

## Features

- **Event Storage**: Store events with unique IDs and descriptive data
- **Causal Relationships**: Link events to their parent events, creating a dependency graph
- **Persistent Storage**: Events are stored in a binary file format
- **Modern Web Frontend**: Beautiful, responsive web interface
- **Real-time Updates**: See changes immediately in the frontend
- **Data Export**: Export your event data as JSON
- **Visual Graph**: View causal relationships in a simple graph format

## Project Structure

```
causaldb/
├── main.c              # Main CLI application
├── db.c/h              # Database operations
├── event.c/h           # Event data structures
├── statement.c/h       # SQL-like statement parsing
├── repl.c/h            # Read-Eval-Print Loop
├── frontend/           # Web frontend
│   ├── index.html      # Main HTML page
│   ├── styles.css      # Modern CSS styling
│   └── script.js       # Frontend JavaScript
├── server/             # HTTP server
│   ├── server.c        # HTTP server implementation
│   └── Makefile        # Server build configuration
└── Makefile            # Main build configuration
```

## Quick Start

### 1. Build the Project

```bash
# Build the main CLI application
make

# Build the HTTP server (for frontend)
cd server
make
cd ..
```

### 2. Run the Web Frontend

```bash
# Start the HTTP server
cd server
./server
```

The frontend will be available at: http://localhost:8080

### 3. Use the CLI (Alternative)

```bash
# Run the command-line interface
./causaldb
```

## Usage

### Web Frontend

1. **Add Events**: Use the form on the left to add new events

   - Enter an Event ID (unique number)
   - Enter event description
   - Optionally specify parent event IDs (space-separated)

2. **View Events**: All events are displayed in the main area

   - Click on any event card to see details
   - Parent relationships are shown with tags

3. **Search Events**: Use the search form to find specific events by ID

4. **Export Data**: Click the export button to download your data as JSON

### CLI Commands

- `.list` - Display all events
- `.exit` - Exit the program
- `insert <id> "<data>" [parent1 parent2 ...]` - Insert a new event
- `get <id>` - Retrieve a specific event

## Example Usage

### Creating a Project Timeline

```bash
# Add initial project setup
insert 1 "Project initialization"

# Add database design (depends on project setup)
insert 2 "Database schema design" 1

# Add API implementation (depends on both previous events)
insert 3 "API endpoint implementation" 1 2

# Add frontend development (depends on API)
insert 4 "Frontend development" 3

# Add testing (depends on API and frontend)
insert 5 "Testing and debugging" 3 4
```

This creates a causal graph:

```
1 (Project initialization)
├── 2 (Database schema design)
│   └── 3 (API endpoint implementation)
│       ├── 4 (Frontend development)
│       └── 5 (Testing and debugging)
```

## API Endpoints

The HTTP server provides these REST endpoints:

- `GET /api/events` - Retrieve all events
- `POST /api/events` - Create a new event

### Example API Usage

```bash
# Get all events
curl http://localhost:8080/api/events

# Add a new event
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{
    "id": 1,
    "data": "Project initialization",
    "parent_count": 0,
    "parents": []
  }'
```

## Data Format

Events are stored with the following structure:

```c
typedef struct {
    uint32_t id;                    // Unique event identifier
    uint8_t parent_count;           // Number of parent events
    uint32_t parents[MAX_PARENTS];  // Array of parent event IDs
    char data[MAX_DATA_LENGTH];     // Event description
} Event;
```

- **ID**: Unique 32-bit unsigned integer
- **Data**: Text description (up to 128 characters)
- **Parents**: Array of parent event IDs (up to 8 parents)
- **Parent Count**: Number of parent events

## Building from Source

### Prerequisites

- GCC compiler
- Make
- Standard C library

### Build Commands

```bash
# Build main application
make

# Build HTTP server
cd server && make && cd ..

# Clean build artifacts
make clean
cd server && make clean && cd ..
```

## Development

### Adding New Features

1. **Backend**: Modify the C files in the root directory
2. **Frontend**: Edit files in the `frontend/` directory
3. **Server**: Update `server/server.c` for new API endpoints

### File Structure Details

- **Database Files**: Events are stored in `causal.cdb` (binary format)
- **Frontend Assets**: Static files served from `frontend/` directory
- **Server**: HTTP server runs on port 8080 by default

## Future Enhancements

- [ ] Advanced graph visualization with D3.js
- [ ] Event filtering and sorting
- [ ] Bulk import/export operations
- [ ] User authentication
- [ ] Real-time collaboration
- [ ] Event templates
- [ ] Advanced search capabilities
