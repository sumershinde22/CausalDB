#!/bin/bash

echo "Testing CausalDB API..."

# Test 1: Get all events (should return empty array initially)
echo "1. Testing GET /api/events..."
curl -s http://localhost:8080/api/events
echo -e "\n"

# Test 2: Add a test event
echo "2. Testing POST /api/events..."
curl -s -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{
    "id": 1,
    "data": "Test event",
    "parent_count": 0,
    "parents": []
  }'
echo -e "\n"

# Test 3: Get all events again (should now include the test event)
echo "3. Testing GET /api/events (after adding event)..."
curl -s http://localhost:8080/api/events
echo -e "\n"

# Test 4: Add another event with a parent
echo "4. Testing POST /api/events with parent..."
curl -s -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{
    "id": 2,
    "data": "Child event",
    "parent_count": 1,
    "parents": [1]
  }'
echo -e "\n"

# Test 5: Get all events again
echo "5. Testing GET /api/events (final state)..."
curl -s http://localhost:8080/api/events
echo -e "\n"

echo "API tests completed!" 