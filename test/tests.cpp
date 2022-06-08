#ifdef __cplusplus
extern "C" {
#endif

#include "RingBufLib.h"

#ifdef __cplusplus
}
#endif


#include <iostream>
#include <string.h>

#define CONFIG_CATCH_MAIN
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark_all.hpp>
#include <catch2/matchers/catch_matchers_all.hpp>


using namespace std;

struct BuffTest_typ {
	int num1;
	int num2;
};

TEST_CASE( "Test RingBufLib BufferInit & BufferDestroy", "[RingBufLib]" ) {
	
	SECTION( "BufferInit should handle invalid inputs" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		BufferInit(0, 10, sizeof(testVal));
		CHECK(buf.Data == 0);
		BufferInit((UDINT)&buf, 0, sizeof(testVal));
		CHECK(buf.Data == 0);
		// BufferInit((UDINT)&buf, 10, 0); // Although this will result in 0 memory its not invalid
		// CHECK(buf.Data == 0);
		BufferInit(0, 0, 0);
		CHECK(buf.Data == 0);

		//BufferDestroy((UDINT)&buf); // Destroy buffer to prevent memory leaks
	}
	
	SECTION( "Should Alloc space properly" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		BufferInit((UDINT)&buf, 10, sizeof(testVal));
		CHECK(buf.Data != 0);
		CHECK(buf.DataSize == sizeof(testVal));
		CHECK(buf.MaxValues == 10);
		CHECK(buf.NumberValues == 0);

		BufferDestroy((UDINT)&buf); // Destroy buffer to prevent memory leaks
	}

	SECTION( "Should Re-Alloc space properly" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		BufferInit((UDINT)&buf, 10, sizeof(testVal));
		CHECK(buf.Data != 0);
		CHECK(buf.DataSize == sizeof(testVal));
		CHECK(buf.MaxValues == 10);
		CHECK(buf.NumberValues == 0);

		BufferInit((UDINT)&buf, 5, 100);
		CHECK(buf.Data != 0);
		CHECK(buf.DataSize == 100);
		CHECK(buf.MaxValues == 5);
		CHECK(buf.NumberValues == 0);

		BufferDestroy((UDINT)&buf); // Destroy buffer to prevent memory leaks
	}

	SECTION( "BufferDestory should handle invalid inputs" ) {
		Buffer_typ buf = {};

		CHECK(BufferDestroy(0) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferDestroy((UDINT)&buf) == RING_BUF_ERR_DATA_NOT_INIT);
	}

	SECTION( "Should De-Alloc when destroyed" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};

		BufferInit((UDINT)&buf, 5, 10);
		buf.NumberValues = 1;
		BufferDestroy((UDINT)&buf);
		CHECK(buf.Data == 0);
		CHECK(buf.DataSize == 0);
		CHECK(buf.MaxValues == 0);
		CHECK(buf.NumberValues == 0);
	}
}

TEST_CASE( "Test RingBufLib Buffer info functions", "[RingBufLib]" ) {
	
	SECTION( "BufferValid" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		CHECK(BufferValid(0) == 0);
		CHECK(BufferValid((UDINT)&buf) == 0);

		BufferInit((UDINT)&buf, 5, 10);
		CHECK(BufferValid((UDINT)&buf) == 1);

		buf.MaxValues = 0; // Force max values to 0
		CHECK(BufferValid((UDINT)&buf) == 0);
		buf.MaxValues = 5; // Restore max values

		BufferDestroy((UDINT)&buf); // Destroy buffer to prevent memory leaks
	}
	SECTION( "BufferStatus" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		CHECK(BufferStatus(0) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferStatus((UDINT)&buf) == RING_BUF_ERR_DATA_NOT_INIT);

		BufferInit((UDINT)&buf, 5, 10);
		CHECK(BufferStatus((UDINT)&buf) == 0);

		buf.MaxValues = 0; // Force max values to 0
		CHECK(BufferStatus((UDINT)&buf) == RING_BUF_ERR_MAX_VALUES_ZERO);
		buf.MaxValues = 5; // Restore max values
	}
	SECTION( "BufferFull" ) {
		Buffer_typ buf = {};
		BuffTest_typ testVal = {};
		
		BufferInit((UDINT)&buf, 1, 1);
		
		CHECK(BufferFull(0) == true);
		CHECK(BufferFull((UDINT)&buf) == false);
		
		buf.NumberValues = 1; // Pretend to add item to buffer
		CHECK(BufferFull((UDINT)&buf) == true);
	}
}

TEST_CASE( "Test RingBufLib Add / Remove items", "[RingBufLib]" ) {
	
	SECTION( "Check all Add / Remove fns for 0 input" ) {
		BuffTest_typ testVal = {};

		CHECK(BufferAddToTop(0, (UDINT)&testVal) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferAddToBottom(0, (UDINT)&testVal) == RING_BUF_ERR_INVALID_BUF_POINTER);
		
		CHECK(BufferRemoveTop(0) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferRemoveBottom(0) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferRemoveOffset(0, 0, 0) == RING_BUF_ERR_INVALID_BUF_POINTER);
		CHECK(BufferClear(0) == RING_BUF_ERR_INVALID_BUF_POINTER);
	}

	SECTION( "Check all Add / Remove fns for un init buffer" ) {
		Buffer_typ buf = {}; // An un initialized buffer
		BuffTest_typ testVal = {};

		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == RING_BUF_ERR_DATA_NOT_INIT);
		CHECK(BufferAddToBottom((UDINT)&buf, (UDINT)&testVal) == RING_BUF_ERR_DATA_NOT_INIT);
		
		CHECK(BufferRemoveTop((UDINT)&buf) == RING_BUF_ERR_DATA_NOT_INIT);
		CHECK(BufferRemoveBottom((UDINT)&buf) == RING_BUF_ERR_DATA_NOT_INIT);
		CHECK(BufferRemoveOffset((UDINT)&buf, 0, 0) == RING_BUF_ERR_DATA_NOT_INIT);
		CHECK(BufferClear((UDINT)&buf) == RING_BUF_ERR_DATA_NOT_INIT);
	}

	SECTION( "Buffer Should be able to add/remove to/from top" ) {
		Buffer_typ buf = {}; // An initialized buffer
		BuffTest_typ testVal = {};

		// Initialize buffer
		BufferInit((UDINT)&buf, 5, sizeof(testVal));
		CHECK(buf.NumberValues == 0);
		CHECK(buf.TopIndex == 0);

		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(buf.NumberValues == 1);
		CHECK(buf.TopIndex == 4);
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(buf.NumberValues == 2);
		CHECK(buf.TopIndex == 3);

		CHECK(BufferRemoveTop((UDINT)&buf) == 0);
		CHECK(buf.NumberValues == 1);
		CHECK(buf.TopIndex == 4);
		CHECK(BufferRemoveTop((UDINT)&buf) == 0);
		CHECK(buf.NumberValues == 0);
		CHECK(buf.TopIndex == 0);

	}

	SECTION( "Buffer Should be able to add/remove to/from bottom" ) {
		Buffer_typ buf = {}; // An initialized buffer
		BuffTest_typ testVal = {};

		// Initialize buffer
		BufferInit((UDINT)&buf, 5, sizeof(testVal));
		CHECK(buf.NumberValues == 0);
		CHECK(buf.TopIndex == 0);

		CHECK(BufferAddToBottom((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(buf.NumberValues == 1);
		CHECK(buf.TopIndex == 0);
		CHECK(BufferAddToBottom((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(buf.NumberValues == 2);
		CHECK(buf.TopIndex == 0);

		CHECK(BufferRemoveBottom((UDINT)&buf) == 0);
		CHECK(buf.NumberValues == 1);
		CHECK(buf.TopIndex == 0);
		CHECK(BufferRemoveBottom((UDINT)&buf) == 0);
		CHECK(buf.NumberValues == 0);
		CHECK(buf.TopIndex == 0);
	}

	SECTION( "Buffer Should be able to remove from offset and clear" ) {
		Buffer_typ buf = {}; // An initialized buffer
		BuffTest_typ testVal = {};

		// Initialize buffer
		BufferInit((UDINT)&buf, 5, sizeof(testVal));
		CHECK(buf.NumberValues == 0);
		CHECK(buf.TopIndex == 0);
		
		// Add 5 values
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(BufferAddToTop((UDINT)&buf, (UDINT)&testVal) == 0);
		CHECK(buf.NumberValues == 5);
		CHECK(buf.TopIndex == 0);

		CHECK(BufferRemoveOffset((UDINT)&buf, 2, 0) == 0);
		CHECK(buf.NumberValues == 4);
		CHECK(buf.TopIndex == 0);

		CHECK(BufferClear((UDINT)&buf) == 0);
		CHECK(buf.NumberValues == 0);
	}
}
