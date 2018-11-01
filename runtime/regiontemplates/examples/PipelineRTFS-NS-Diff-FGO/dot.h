#ifndef DOT_H
#define DOT_H

void DEBUG_PCBLIST_TO_DOT(const std::string& filename,
						  const std::map<int, PipelineComponentBase*>& stages) {
	std::ofstream os(filename + ".dot");

	os << "digraph graphname {" << std::endl;
	os << "graph[compound=true];\nrankdir=\"LR\";" << std::endl;

	for (std::pair<const int, PipelineComponentBase*> s : stages) {
		os << "subgraph cluster" << s.second->getId() << " {" << std::endl;
		os << "label = \"Stage " << s.second->getId() << "\";" << std::endl;
		for (ReusableTask* t : s.second->tasks) {
			os << t->getId() << " [shape=box,label=\"" << t->getId() << "\\n"
			   << t->getTaskName() << "\"]; " << std::endl;

			if(t->parentTask != -1)
			{
				os << t->parentTask << " -> " << t->getId() << ";\n";
				if(t->parentTask != t->fakeParent)
				{
					os << t->fakeParent << " -> " << t->getId() << " [color=blue];\n";
				}
			}
		}
		os << 's' << s.second->getId() << " [shape=plaintext,label=\"\"]; "
		   << std::endl;
		os << "}" << std::endl;
		if (auto reused = s.second->reused) {
			// os << 's' << s.second->getId() << " [shape=plaintext,label=\"" <<
			// s.second->getId() << "\\nReused:" << s.second->reused->getId();
			// os
			// << "\"]; " << std::endl;
			os << 's' << s.second->getId() << " -> s"
			   << s.second->reused->getId();
			os << " [ltail=cluster" << s.second->getId() << " lhead=cluster"
			   << s.second->reused->getId() << "];" << std::endl;
		}
	}
	os << "}" << std::endl;
}

void DEBUG_PCB_TO_DOT(const std::string& filename,
					  PipelineComponentBase* stage) {
	std::ofstream os(filename + std::to_string(stage->getId()) + ".dot");

	os << "digraph graphname {" << std::endl;
	os << "subgraph cluster" << stage->getId() << " {" << std::endl;
	os << "label = \"Bucket " << stage->getId() << "\";" << std::endl;
	for (ReusableTask* t : stage->tasks) {
		os << t->getId() << " [shape=box,label=\"" << t->getId() << "\\n"
		   << t->getTaskName() << "\"]; " << std::endl;

		if(t->parentTask != -1)
		{
			os << t->parentTask << " -> " << t->getId() << ";\n";
			if(t->parentTask != t->fakeParent)
			{
				os << t->fakeParent << " -> " << t->getId() << " [color=blue];\n";
			}
		}
	}
	os << "}" << std::endl;
	os << "}" << std::endl;
}

#endif
