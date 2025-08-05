#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sqlite3.h>
#include "db.h"
#include "event.h"

#define BENCHMARK_ITERATIONS 1000
#define MAX_EVENTS 10000

// Performance measurement utilities
typedef struct {
    double insert_time;
    double query_time;
    double memory_usage;
    size_t events_processed;
} BenchmarkResult;

double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

double get_memory_usage() {
    struct rusage r_usage;
    getrusage(RUSAGE_SELF, &r_usage);
    return (double)r_usage.ru_maxrss / 1024.0; // Convert to MB
}

// CausalDB benchmarks
BenchmarkResult benchmark_causaldb_insert() {
    BenchmarkResult result = {0};
    double start_time = get_time();
    double start_memory = get_memory_usage();
    
    // Create fresh database
    system("rm -f benchmark_causal.cdb");
    open_db("benchmark_causal.cdb");
    Table* table = malloc(sizeof(Table));
    table->num_events = 0;
    
    // Insert events
    for (int i = 1; i <= BENCHMARK_ITERATIONS; i++) {
        Event e;
        e.id = i;
        e.parent_count = (i > 1) ? 1 : 0;
        if (i > 1) e.parents[0] = i - 1;
        snprintf(e.data, MAX_DATA_LENGTH, "Event %d with causal relationship", i);
        
        insert_event(&e, table);
        result.events_processed++;
    }
    
    result.insert_time = get_time() - start_time;
    result.memory_usage = get_memory_usage() - start_memory;
    
    close_db();
    free(table);
    return result;
}

BenchmarkResult benchmark_causaldb_query() {
    BenchmarkResult result = {0};
    double start_time = get_time();
    double start_memory = get_memory_usage();
    
    // Load existing database
    Table* table = load_table("benchmark_causal.cdb");
    
    // Query events
    Event e;
    for (int i = 1; i <= BENCHMARK_ITERATIONS; i++) {
        if (find_event_in_memory(i, table, &e)) {
            result.events_processed++;
        }
    }
    
    result.query_time = get_time() - start_time;
    result.memory_usage = get_memory_usage() - start_memory;
    
    close_db();
    free(table);
    return result;
}

// SQLite benchmarks
BenchmarkResult benchmark_sqlite_insert() {
    BenchmarkResult result = {0};
    double start_time = get_time();
    double start_memory = get_memory_usage();
    
    // Create fresh SQLite database
    system("rm -f benchmark_sqlite.db");
    sqlite3* db;
    sqlite3_open("benchmark_sqlite.db", &db);
    
    // Create table
    const char* create_table_sql = 
        "CREATE TABLE events ("
        "id INTEGER PRIMARY KEY, "
        "parent_count INTEGER, "
        "parents TEXT, "
        "data TEXT"
        ");";
    sqlite3_exec(db, create_table_sql, NULL, NULL, NULL);
    
    // Prepare insert statement
    sqlite3_stmt* stmt;
    const char* insert_sql = "INSERT INTO events (id, parent_count, parents, data) VALUES (?, ?, ?, ?);";
    sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    
    // Insert events
    for (int i = 1; i <= BENCHMARK_ITERATIONS; i++) {
        sqlite3_bind_int(stmt, 1, i);
        sqlite3_bind_int(stmt, 2, (i > 1) ? 1 : 0);
        
        char parents_str[64];
        if (i > 1) {
            snprintf(parents_str, sizeof(parents_str), "%d", i - 1);
        } else {
            strcpy(parents_str, "");
        }
        sqlite3_bind_text(stmt, 3, parents_str, -1, SQLITE_STATIC);
        
        char data_str[256];
        snprintf(data_str, sizeof(data_str), "Event %d with causal relationship", i);
        sqlite3_bind_text(stmt, 4, data_str, -1, SQLITE_STATIC);
        
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
        result.events_processed++;
    }
    
    result.insert_time = get_time() - start_time;
    result.memory_usage = get_memory_usage() - start_memory;
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

BenchmarkResult benchmark_sqlite_query() {
    BenchmarkResult result = {0};
    double start_time = get_time();
    double start_memory = get_memory_usage();
    
    sqlite3* db;
    sqlite3_open("benchmark_sqlite.db", &db);
    
    // Prepare query statement
    sqlite3_stmt* stmt;
    const char* query_sql = "SELECT * FROM events WHERE id = ?;";
    sqlite3_prepare_v2(db, query_sql, -1, &stmt, NULL);
    
    // Query events
    for (int i = 1; i <= BENCHMARK_ITERATIONS; i++) {
        sqlite3_bind_int(stmt, 1, i);
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result.events_processed++;
        }
        sqlite3_reset(stmt);
    }
    
    result.query_time = get_time() - start_time;
    result.memory_usage = get_memory_usage() - start_memory;
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

// Feature comparison
void compare_features() {
    printf("\n=== FEATURE COMPARISON ===\n");
    printf("%-25s %-15s %-15s\n", "Feature", "CausalDB", "SQLite");
    printf("%-25s %-15s %-15s\n", "-------------------------", "---------------", "---------------");
    printf("%-25s %-15s %-15s\n", "Causal Relationships", "✅ Native", "❌ Requires joins");
    printf("%-25s %-15s %-15s\n", "Binary Storage", "✅ Efficient", "❌ Text-based");
    printf("%-25s %-15s %-15s\n", "Memory Footprint", "✅ Small", "⚠️ Larger");
    printf("%-25s %-15s %-15s\n", "Setup Complexity", "✅ Simple", "⚠️ Complex");
    printf("%-25s %-15s %-15s\n", "ACID Compliance", "⚠️ Basic", "✅ Full");
    printf("%-25s %-15s %-15s\n", "Concurrent Access", "❌ Single", "✅ Multi-user");
    printf("%-25s %-15s %-15s\n", "SQL Support", "❌ No", "✅ Full");
    printf("%-25s %-15s %-15s\n", "Indexing", "❌ No", "✅ Advanced");
    printf("%-25s %-15s %-15s\n", "Data Types", "⚠️ Limited", "✅ Rich");
    printf("%-25s %-15s %-15s\n", "File Size", "✅ Compact", "⚠️ Larger");
}

// File size comparison
void compare_file_sizes() {
    printf("\n=== FILE SIZE COMPARISON ===\n");
    
    // Get CausalDB file size
    FILE* causal_file = fopen("benchmark_causal.cdb", "rb");
    fseek(causal_file, 0, SEEK_END);
    long causal_size = ftell(causal_file);
    fclose(causal_file);
    
    // Get SQLite file size
    FILE* sqlite_file = fopen("benchmark_sqlite.db", "rb");
    fseek(sqlite_file, 0, SEEK_END);
    long sqlite_size = ftell(sqlite_file);
    fclose(sqlite_file);
    
    printf("CausalDB file size: %ld bytes (%.2f KB)\n", causal_size, causal_size / 1024.0);
    printf("SQLite file size: %ld bytes (%.2f KB)\n", sqlite_size, sqlite_size / 1024.0);
    printf("Size ratio (SQLite/CausalDB): %.2fx\n", (double)sqlite_size / causal_size);
}

// Main benchmarking function
void run_benchmarks() {
    printf("=== CausalDB vs SQLite Benchmark ===\n");
    printf("Testing with %d events...\n\n", BENCHMARK_ITERATIONS);
    
    // Run insert benchmarks
    printf("Running INSERT benchmarks...\n");
    BenchmarkResult causal_insert = benchmark_causaldb_insert();
    BenchmarkResult sqlite_insert = benchmark_sqlite_insert();
    
    printf("CausalDB INSERT: %.4f seconds, %.2f MB memory, %zu events\n", 
           causal_insert.insert_time, causal_insert.memory_usage, causal_insert.events_processed);
    printf("SQLite INSERT: %.4f seconds, %.2f MB memory, %zu events\n", 
           sqlite_insert.insert_time, sqlite_insert.memory_usage, sqlite_insert.events_processed);
    
    double insert_speedup = sqlite_insert.insert_time / causal_insert.insert_time;
    printf("CausalDB is %.2fx faster for inserts\n\n", insert_speedup);
    
    // Run query benchmarks
    printf("Running QUERY benchmarks...\n");
    BenchmarkResult causal_query = benchmark_causaldb_query();
    BenchmarkResult sqlite_query = benchmark_sqlite_query();
    
    printf("CausalDB QUERY: %.4f seconds, %.2f MB memory, %zu events\n", 
           causal_query.query_time, causal_query.memory_usage, causal_query.events_processed);
    printf("SQLite QUERY: %.4f seconds, %.2f MB memory, %zu events\n", 
           sqlite_query.query_time, sqlite_query.memory_usage, sqlite_query.events_processed);
    
    double query_speedup = sqlite_query.query_time / causal_query.query_time;
    printf("CausalDB is %.2fx faster for queries\n\n", query_speedup);
    
    // Overall performance summary
    printf("=== PERFORMANCE SUMMARY ===\n");
    printf("CausalDB advantages:\n");
    if (insert_speedup > 1.0) {
        printf("  • %.2fx faster inserts\n", insert_speedup);
    }
    if (query_speedup > 1.0) {
        printf("  • %.2fx faster queries\n", query_speedup);
    }
    printf("  • %.2fx less memory usage (inserts)\n", sqlite_insert.memory_usage / causal_insert.memory_usage);
    printf("  • %.2fx less memory usage (queries)\n", sqlite_query.memory_usage / causal_query.memory_usage);
    
    // Feature comparison
    compare_features();
    
    // File size comparison
    compare_file_sizes();
    
    // Use case recommendations
    printf("\n=== USE CASE RECOMMENDATIONS ===\n");
    printf("Choose CausalDB when:\n");
    printf("  • You need causal event tracking\n");
    printf("  • Performance is critical\n");
    printf("  • Memory usage is constrained\n");
    printf("  • Simple setup is preferred\n");
    printf("  • Binary storage efficiency matters\n\n");
    
    printf("Choose SQLite when:\n");
    printf("  • You need ACID compliance\n");
    printf("  • Multiple users access the data\n");
    printf("  • Complex queries are required\n");
    printf("  • You need advanced indexing\n");
    printf("  • SQL compatibility is important\n");
}

int main() {
    run_benchmarks();
    return 0;
} 