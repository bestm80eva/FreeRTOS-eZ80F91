	dcl
		memptr entry         returns (ptr),
		memsiz entry         returns (fixed(15)),
		memwds entry         returns (fixed(15)),
		dfcb0  entry         returns (ptr),
		dfcb1  entry         returns (ptr),
		dbuff  entry         returns (ptr),
		reboot entry,
		rdcon  entry         returns (char(1)),
		wrcon  entry         (char(1)),
		rdrdr  entry         returns (char(1)),
		wrpun  entry         (char(1)),
		wrlst  entry         (char(1)),
		coninp entry         returns (char(1)),
		conout entry         (char(1)),
		rdstat entry         returns (bit(1)),
		getio  entry         returns (bit(8)),
		setio  entry         (bit(8)),
		wrstr  entry         (ptr),
		rdbuf  entry         (ptr),
		break  entry         returns (bit(1)),
		vers   entry         returns (bit(16)),
		reset  entry,
		select entry         (fixed(7)),
		open   entry   (ptr) returns (fixed(7)),
		close  entry   (ptr) returns (fixed(7)),
		sear   entry   (ptr) returns (fixed(7)),
		searn  entry         returns (fixed(7)),
		delete entry   (ptr),
		rdseq  entry   (ptr) returns (fixed(7)),
		wrseq  entry   (ptr) returns (fixed(7)),
		make   entry   (ptr) returns (fixed(7)),
		rename entry   (ptr),
		logvec entry         returns (bit(16)),
		curdsk entry         returns (fixed(7)),
		setdma entry         (ptr),
		allvec entry         returns (ptr),
		wpdisk entry,
		rovec  entry         returns (bit(16)),
		filatt entry         (ptr),
		getdpb entry         returns (ptr),
		getusr entry         returns (fixed(7)),
		setusr entry   (fixed(7)),
		rdran  entry   (ptr) returns (fixed(7)),
		wrran  entry   (ptr) returns (fixed(7)),
		filsiz entry   (ptr),
		setrec entry   (ptr),
		resdrv entry         (bit(16)),
		wrranz entry   (ptr) returns (fixed(7));
