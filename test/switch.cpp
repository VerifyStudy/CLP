bool Switchtoif(Stmt *s)
	{
		if(isa<SwitchStmt>(s))
		{
			SwitchStmt *switchStatement = cast<SwitchStmt>(s);
			SourceRange sourceRange;

			//make a string from the switch condition 
			Expr *condExpr = switchStatement->getCond();
			bool invalid;
			std::string condition =  Lexer::getSourceText(CharSourceRange(condExpr->getSourceRange(), true),
														TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid).str();
			//find_if函数提供了一种对数组、STL容器进行查找的方法,
			//用于在指定区域内执行查找操作
			bool isVariable = find_if(condition.begin(), condition.end(),
						[](char c) { return !(isalnum(c)); }) == condition.end();
			if(!isVariable){
				condition.insert(0, 1, '(').append(")");
			}
			condition.append(" == ");
			
			SwitchCase *caseList = switchStatement->getSwitchCaseList();
			//checking if the switch has a default case
			bool hasDefault = false;
			sourceRange.setBegin(caseList->getKeywordLoc());
			sourceRange.setEnd(caseList->getKeywordLoc().getLocWithOffset(6));
			std::string lastCase = Lexer::getSourceText(CharSourceRange(sourceRange, true),
						   TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid).str();
			if(lastCase.compare("default") == 0){
				hasDefault = true;
			}

			//creating a vector of statement strings
			//Note: starts from bottom to top case
			Stmt *body = switchStatement->getBody();
			sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
			sourceRange.setEnd(body->getEndLoc().getLocWithOffset(-1));
			std::vector<std::string> caseStatements;

			caseStatements.push_back(Lexer::getSourceText(CharSourceRange(sourceRange, true),
				 				 TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid).str());
			
			while(caseList->getNextSwitchCase() != nullptr){
				sourceRange.setBegin(caseList->getNextSwitchCase()->getColonLoc().getLocWithOffset(1));
				sourceRange.setEnd(caseList->getKeywordLoc().getLocWithOffset(-1));
				caseStatements.emplace_back( Lexer::getSourceText(CharSourceRange(sourceRange, true),
					 				TheCompiler.getSourceManager(), TheCompiler.getLangOpts(), &invalid).str());
				caseList = caseList->getNextSwitchCase();
			}

			//Note: for fall throughs we need to move from right to left
			//to respect the case order
			int i;
			for(i = caseStatements.size()-1; i>=0; i--){
				if(!hasBreak(caseStatements[i])){
					for(int j=i-1; j>=0; j--){
						if(hasBreak(caseStatements[j])){
							caseStatements[i].append(caseStatements[j]);
							eraseBreak(caseStatements[i]);
							break;
						}
						caseStatements[i].append(caseStatements[j]);
					}
				}
				else
					eraseBreak(caseStatements[i]);
			}

			//adding if-elseif-else
			//reseting back after making the vector
			caseList = switchStatement->getSwitchCaseList();
			//Note: we are moving through the vector from
			//left to right when falling through
			bool defaultCaseRewritten = false;
			int numberOfStmts=caseStatements.size();
			i=0;
			SourceLocation nextCaseLoc = body->getEndLoc();

			while(caseList != nullptr && i<numberOfStmts){
				if(hasDefault == true && defaultCaseRewritten == false){
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), "}else if{");
					TheRewriter.ReplaceText(caseList->getKeywordLoc(), 7, "");
					TheRewriter.ReplaceText(caseList->getColonLoc(), 1, "");

					sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
					sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
					//indentCaseStmt(caseStatements[i]);
					TheRewriter.ReplaceText(sourceRange, caseStatements[i]);

					defaultCaseRewritten = true;
					nextCaseLoc = caseList->getKeywordLoc();
					caseList = caseList->getNextSwitchCase();
					i++;
					continue;
				}
				if(caseList->getNextSwitchCase() == nullptr){
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), std::string{"if("}.append(condition));
				}else{
					TheRewriter.InsertTextBefore(caseList->getKeywordLoc(), std::string{"}else if("}.append(condition));
				}
				TheRewriter.ReplaceText(caseList->getKeywordLoc(), 4, "");
				TheRewriter.ReplaceText(caseList->getColonLoc(), 1, "){\n");

				//indentCaseStmt(caseStatements[i]);
				sourceRange.setBegin(caseList->getColonLoc().getLocWithOffset(1));
				sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
				TheRewriter.ReplaceText(sourceRange, caseStatements[i]);

				nextCaseLoc = caseList->getKeywordLoc();
				caseList = caseList->getNextSwitchCase();
				i++;
			}
			//removing space from switch to the first case
			sourceRange.setBegin(switchStatement->getSwitchLoc());
			sourceRange.setEnd(nextCaseLoc.getLocWithOffset(-1));
			TheRewriter.RemoveText(sourceRange);
		
		}//if (isa<switchstmt>(s))
		return true;
	}//bool Switchtoif(Stmt *s)
