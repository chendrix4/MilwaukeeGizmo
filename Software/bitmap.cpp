#include "Arduino.h"
#include "bitmap.h"

void Bitmap::loadSDBitmap(const char * filename) {
	// Get necessary data from file and determine if bitmap can be drawn

	if ((file = SD.open(filename)) == NULL)
		return;

	// Err 1 : Bitmap signature not found
	if (readBytes<uint16_t>() != 0x4D42)
		return;

	(void)readBytes<uint32_t>(); // File size
	(void)readBytes<uint32_t>(); // Creator bytes
	imageOffset = readBytes<uint32_t>();
	(void)readBytes<uint32_t>(); // Header size
	width = readBytes<uint32_t>();
	height = readBytes<uint32_t>();

	// Err 2 : # of planes != 1
	if (readBytes<uint16_t>() != 1) // # of planes
		return;

	// Err 3 : Depth != 24
	if (readBytes<uint16_t>() != 24)
		return;

	// Err 4 : Uncompressed
	if (readBytes<uint32_t>() != 0)
		return;

}

void Bitmap::setBitmapWindow(uint8_t x, uint8_t y) {
	// Setup drawing window for bitmap image

	// Padded (if needed) to 4-byte boundary
	rowSize = (width * 3 + 3) & ~3;

	// If bmpHeight is negative, image is in top-down order.
	// This is not canon but has been observed in the wild.
	flip = true;
	if (height < 0) {
		height = -height;
		flip = false;
	}

	// Crop area to be loaded
	if ((x + width - 1) >= disp.width())
		width = disp.width() - x;
	if ((y + height - 1) >= disp.height())
		height = disp.height() - y;

	// Set TFT address window to clipped image bounds
	disp.setAddrWindow(x, y, x + width - 1, y + height - 1);
}

void Bitmap::drawSDBitmap(const char * filename, uint8_t x, uint8_t y) {

	uint8_t sdbuffer[3 * BUFFPIXEL];
	uint8_t buffidx = sizeof(sdbuffer);
	uint8_t r, g, b;
	uint32_t pos = 0;

	loadSDBitmap(filename);
	setBitmapWindow(x, y);

	for (int row = 0; row<height; row++) { // For each scanline...
										   // Seek to start of scan line.  It might seem labor-
										   // intensive to be doing this on every line, but this
										   // method covers a lot of gritty details like cropping
										   // and scanline padding.  Also, the seek only takes
										   // place if the file position actually needs to change
										   // (avoids a lot of cluster math in SD library).

		if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
			pos = imageOffset + (height - 1 - row) * rowSize;
		else     // Bitmap is stored top-to-bottom
			pos = imageOffset + row * rowSize;

		if (file.position() != pos) { // Need seek?
			file.seek(pos);
			buffidx = sizeof(sdbuffer); // Force buffer reload
		}

		for (int col = 0; col<width; col++) { // For each pixel...
											  // Time to read more pixel data?
			if (buffidx >= sizeof(sdbuffer)) { // Indeed
				file.read(sdbuffer, sizeof(sdbuffer));
				buffidx = 0; // Set index to beginning
			}

			// Convert pixel from BMP to TFT format, push to display
			b = sdbuffer[buffidx++];
			g = sdbuffer[buffidx++];
			r = sdbuffer[buffidx++];
			disp.pushColor(disp.Color565(r, g, b));
		} // end pixel
	} // end scanline

	file.close();
}
